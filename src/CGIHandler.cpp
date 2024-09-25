#include "CGIHandler.hpp"

/* ====== CONSTRUCTOR/DESTRUCTOR ====== */


CGIHandler::CGIHandler(Connection& connection)
	: m_cgiPath(connection.location->cgiPath)
	, m_cgiExt(connection.location->cgiExt)
	, m_pipeIn()
	, m_pipeOut()
	, m_cgiPid(-1)
{

	// Set up environment variables for CGI script
	if (connection.m_request.headers.find("Content-Length") != connection.m_request.headers.end())
		m_env["CONTENT_LENGTH"] = connection.m_request.headers.at("Content-Length");
	if (connection.m_request.headers.find("Content-Type") != connection.m_request.headers.end())
		m_env["CONTENT_TYPE"] = connection.m_request.headers.at("Content-Type");
	m_env["GATEWAY_INTERFACE"] = "CGI/1.1";
	m_env["PATH_INFO"] = extractPathInfo(connection.m_request.uri.path); // additional path information following the script name
	m_env["PATH_TRANSLATED"] = connection.location->root + m_env["PATH_INFO"]; // actual file system path of the PATH_INFO resource
	m_env["QUERY_STRING"] = connection.m_request.uri.query;
	m_env["REDIRECT_STATUS"]
		= "200"; // indicates successfull internal redirection to the script; required for old PHP scripts
	m_env["REMOTE_ADDR"] = connection.m_clientSocket.host; // IP address of the client
	m_env["REMOTE_PORT"] = connection.m_clientSocket.port; // port of the client
	m_env["REQUEST_METHOD"] = webutils::methodToString(connection.m_request.method);
	m_env["REQUEST_URI"] = connection.m_request.uri.path + '?' + connection.m_request.uri.query;
	m_env["SCRIPT_NAME"] = extractScriptPath(connection.m_request.uri.path); // URI path of the CGI script until the script extension
	m_env["SCRIPT_FILENAME"]
		= connection.location->root + m_env["SCRIPT_NAME"]; // actual file system path of the CGI script being executed
	m_env["SERVER_ADDR"] = connection.m_serverSocket.host; // IP address of the server
	m_env["SERVER_NAME"] = connection.m_serverSocket.host; // name of the server
	m_env["SERVER_PORT"] = connection.m_serverSocket.port; // port of the server
	m_env["SERVER_PROTOCOL"] = "HTTP/1.1";
	m_env["SERVER_SOFTWARE"] = "Trihard/1.0.0";
	m_env["SYSTEM_ROOT"] = connection.location->root;

	// Create pipes for inter-process communication
	if (pipe(m_pipeIn) == -1) {
		LOG_ERROR << "pipe(): m_pipeIn: " + std::string(std::strerror(errno));
		connection.m_request.httpStatus = StatusInternalServerError;
	}
	if (pipe(m_pipeOut) == -1) {
		LOG_ERROR << "pipe(): pipeOut: " + std::string(std::strerror(errno));
		close(m_pipeIn[0]);
		close(m_pipeIn[1]);
		connection.m_request.httpStatus = StatusInternalServerError;
	}
}

/* ====== GETTER/SETTER FUNCTIONS ====== */

/**
 * @brief Retrieves the write end of the input pipe for the CGI handler.
 *
 * This function returns the file descriptor for the write end of the input pipe
 * (`m_pipeIn`), which is used to send data to the CGI process.
 *
 * @return int The file descriptor corresponding to the write end of the input pipe.
 */
int CGIHandler::getPipeInWriteEnd() const { return m_pipeIn[1]; }

/**
 * @brief Retrieves the read end of the output pipe for the CGI handler.
 *
 * This function returns the file descriptor for the read end of the output pipe
 * (`m_pipeOut`), which is used to read data from the CGI process.
 *
 * @return int The file descriptor corresponding to the read end of the output pipe.
 */
int CGIHandler::getPipeOutReadEnd() const { return m_pipeOut[0]; }

/**
 * @brief Retrieves the process ID (PID) of the CGI handler.
 *
 * This function returns a constant reference to the process ID (`pid_t`) of the CGI process.
 * The PID uniquely identifies the running CGI process.
 *
 * @return const pid_t& A constant reference to the process ID of the CGI process.
 */
const pid_t& CGIHandler::getCGIPid() const { return m_cgiPid; }

/**
 * @brief Retrieves the path to the CGI interpreter.
 *
 * This function returns a constant reference to the path of the CGI interpreter
 * used by the CGI handler. The CGI interpreter is the program that executes the
 * CGI script.
 *
 * @return const std::string& A constant reference to the path of the CGI interpreter.
 */
const std::string& CGIHandler::getCGIPath() const { return m_cgiPath; }

/**
 * @brief Retrieves the file extension associated with the CGI script.
 *
 * This function returns a constant reference to the file extension used to identify
 * CGI scripts. The extension is typically used to determine how the server should
 * handle the script.
 *
 * @return const std::string& A constant reference to the file extension associated with CGI scripts.
 */
const std::string& CGIHandler::getCGIExt() const { return m_cgiExt; }

/**
 * @brief Retrieves the environment variables set for the CGI script.
 *
 * This function returns a constant reference to the environment variables
 * set for the CGI script. The environment variables are used to provide
 * information and configuration to the CGI script during execution.
 *
 * @return const std::map<std::string, std::string>& A constant reference to the environment variables.
 */
const std::map<std::string, std::string>& CGIHandler::getEnv() const { return m_env; }

/* ====== MEMBER FUNCTIONS ====== */

/**
 * @brief Executes the CGI script for the given HTTP request.
 *
 * This function sets up the environment and argument vectors for the CGI script,
 * creates pipes for inter-process communication, and forks a child process to
 * execute the CGI script. The parent process handles the communication with the
 * child process through the pipes.
 *
 * The function performs the following steps:
 * 1. Sets up the environment and argument vectors.
 * 2. Creates pipes for communication between the parent and child processes.
 * 3. Forks a child process.
 * 4. In the child process:
 *    - Redirects stdin and stdout to the appropriate pipe ends.
 *    - Closes unused pipe ends.
 *    - Executes the CGI script using execve().
 *    - If execve() fails, writes an HTTP 500 Internal Server Error response.
 * 5. In the parent process:
 *    - Closes unused pipe ends.
 *
 * If any error occurs during the setup or execution, the function logs the error
 * and sets the HTTP status of the request to 500 Internal Server Error.
 * @param request The HTTP request to be processed by the CGI script.
 */
void CGIHandler::execute(HTTPRequest& request, std::vector<Location>::const_iterator& location)
{
	std::vector<std::string> bufferEnv;
	std::vector<std::string> bufferArgv;
	std::vector<char*> envp;
	std::vector<char*> argv;

	setEnvp(bufferEnv, envp);
	setArgv(bufferArgv, argv);

	m_cgiPid = fork();
	if (m_cgiPid == -1) {
		LOG_ERROR << "Error: fork(): " + std::string(std::strerror(errno));
		close(m_pipeIn[0]);
		close(m_pipeIn[1]);
		close(m_pipeOut[0]);
		close(m_pipeOut[1]);
		request.httpStatus = StatusInternalServerError;
		return;
	}
	if (m_cgiPid == 0) {
		if (!registerChildSignals()) {
			std::string error = "HTTP/1.1 500 Internal Server Error\r\n";
			write(STDOUT_FILENO, error.c_str(), error.size());
			std::exit(EXIT_FAILURE);
		}
		dup2(m_pipeIn[0], STDIN_FILENO); // Replace child stdin with read end of input pipe
		dup2(m_pipeOut[1], STDOUT_FILENO); // Replace child stdout with write end of output pipe
		close(m_pipeIn[0]); // Can be closed as the read connection to server exists in stdin now
		close(m_pipeIn[1]);
		close(m_pipeOut[0]);
		close(m_pipeOut[1]); // Can be closed as the write connection to server exists in stdout now

		std::string workingDir = location->root + location->path;
		if (chdir(workingDir.c_str()) == -1) {
			LOG_ERROR << "Error: chdir(): " + std::string(std::strerror(errno));
			std::string error = "HTTP/1.1 500 Internal Server Error\r\n";
			write(STDOUT_FILENO, error.c_str(), error.size());
			std::exit(EXIT_FAILURE);
		}

		int ret = 0;
		ret = execve(argv[0], argv.data(), envp.data());
		LOG_ERROR << "Error: execve(): " + std::string(std::strerror(errno));
		if (ret == -1) {
			std::string error = "HTTP/1.1 500 Internal Server Error\r\n";
			write(STDOUT_FILENO, error.c_str(), error.size());
			std::exit(EXIT_FAILURE);
		}
	}
	close(m_pipeIn[0]); // Close read end of input pipe in parent process
	close(m_pipeOut[1]); // Close write end of output pipe in parent process
}

/**
 * @brief Sets up the environment variables for the CGI script.
 *
 * This function takes the environment variables stored in the member variable `m_env`,
 * formats them as one string, and stores them in the provided `bufferEnv` vector. It then
 * converts these strings to character pointers and stores them in the `envp` vector,
 * which is used by the CGI script.  It also ensures that the last element
 * of envp is a NULL pointer, as required by exec family functions.
 *
 * @param bufferEnv A reference to a vector of strings where the formatted environment
 *                  variables are stored.
 * @param envp A reference to a vector of character pointers where the addresses of the
 *             formatted environment variables are stored. This vector is used
 *             by the CGI script.
 */
void CGIHandler::setEnvp(std::vector<std::string>& bufferEnv, std::vector<char*>& envp) const
{
	for (std::map<std::string, std::string>::const_iterator citer = m_env.begin(); citer != m_env.end(); citer++) {
		std::string tmp = citer->first + '=' + citer->second;
		bufferEnv.push_back(tmp);
	}
	for (std::vector<std::string>::iterator iter = bufferEnv.begin(); iter != bufferEnv.end(); iter++)
		envp.push_back(&(*iter).at(0));
	envp.push_back(NULL);
}

/**
 * @brief Sets up the argument vector (argv) for the CGI script execution.
 *
 * This function populates the provided argv vector with pointers to the
 * strings in the bufferArgv vector. It also ensures that the last element
 * of argv is a NULL pointer, as required by exec family functions.
 *
 * @param bufferArgv A reference to a vector of strings that is used
 *                   to populate the argv vector.
 * @param argv A reference to a vector of char pointers that is
 *             populated with pointers to the strings in bufferArgv.
 */
void CGIHandler::setArgv(std::vector<std::string>& bufferArgv, std::vector<char*>& argv)
{
	bufferArgv.push_back(m_cgiPath);
	bufferArgv.push_back(m_env["SCRIPT_FILENAME"]);
	for (std::vector<std::string>::iterator iter = bufferArgv.begin(); iter != bufferArgv.end(); iter++)
		argv.push_back(&(*iter).at(0));
	argv.push_back(NULL);
}

// HELPER FUNCTIONS

/**
 * @brief Extracts the path information from a given path based on the CGI extension.
 *
 * This function searches for the CGI extension within the provided path. If the extension
 * is found, it then looks for the first occurrence of a '/' character after the extension.
 * The substring starting from this '/' character is considered the path information and is returned.
 * If the CGI extension or the '/' character is not found, an empty string is returned.
 *
 * @param path The full path from which to extract the path information.
 * @return A string containing the path information if found, otherwise an empty string.
 */
std::string CGIHandler::extractPathInfo(const std::string& path)
{
	size_t extensionStart = path.find(m_cgiExt);
	if (extensionStart == std::string::npos)
		return "";
	std::string tmp = path.substr(extensionStart);
	size_t pathInfoStart = tmp.find('/');
	if (pathInfoStart == std::string::npos)
		return "";
	return tmp.substr(pathInfoStart);
}

/**
 * @brief Extracts the script path from the given path based on the CGI extension.
 *
 * This function searches for the CGI extension within the provided path and
 * returns the substring from the beginning of the path up to and including the
 * CGI extension. If the CGI extension is not found, an empty string is returned.
 *
 * @param path The full path from which to extract the script path.
 * @return std::string The extracted script path or an empty string if the CGI extension is not found.
 */
std::string CGIHandler::extractScriptPath(const std::string& path)
{
	size_t extensionStart = path.find(m_cgiExt);
	if (extensionStart == std::string::npos)
		return "";
	return path.substr(0, extensionStart + m_cgiExt.size());
}

/**
 * @brief Registers signal handlers for various signals.
 *
 * This function sets the default signal handlers for SIGINT, SIGTERM, and SIGQUIT,
 * and ignores the SIGHUP signal. If any of the signal registrations fail, an error
 * message is logged and the function returns false.
 *
 * @return true if all signal handlers are successfully registered, false otherwise.
 */
bool registerChildSignals()
{
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast, performance-no-int-to-ptr)
	if (std::signal(SIGINT, SIG_DFL) == SIG_ERR) {
		LOG_ERROR << "failed to register " << signalNumToName(SIGINT) << ": " << std::strerror(errno);
		return (false);
	}

	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast, performance-no-int-to-ptr)
	if (std::signal(SIGTERM, SIG_DFL) == SIG_ERR) {
		LOG_ERROR << "failed to register " << signalNumToName(SIGTERM) << ": " << std::strerror(errno);
		return (false);
	}

	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast, performance-no-int-to-ptr)
	if (std::signal(SIGHUP, SIG_IGN) == SIG_ERR) {
		LOG_ERROR << "failed to register " << signalNumToName(SIGHUP) << ": " << std::strerror(errno);
		return (false);
	}

	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast, performance-no-int-to-ptr)
	if (std::signal(SIGQUIT, SIG_DFL) == SIG_ERR) {
		LOG_ERROR << "failed to register " << signalNumToName(SIGQUIT) << ": " << std::strerror(errno);
		return (false);
	}
	return (true);
}
