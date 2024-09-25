#include "CGIHandler.hpp"
#include <vector>

/* ====== CONSTRUCTOR/DESTRUCTOR ====== */


/**
 * @brief Constructs a CGIHandler object and sets up the environment for the CGI script.
 * 
 * This constructor initializes the CGIHandler with the provided connection object. It sets up
 * various environment variables required for the CGI script, creates pipes for inter-process
 * communication, and prepares input parameters for the execve system call.
 * 
 * @param connection Reference to a Connection object containing details about the current request and connection.
 * 
 * Environment variables set:
 * - CONTENT_LENGTH: Length of the content, if provided in the request headers.
 * - CONTENT_TYPE: Type of the content, if provided in the request headers.
 * - GATEWAY_INTERFACE: Version of the CGI interface.
 * - PATH_INFO: Additional path information following the script name.
 * - PATH_TRANSLATED: Actual file system path of the PATH_INFO resource.
 * - QUERY_STRING: Query string following the URI path.
 * - REDIRECT_STATUS: Indicates successful internal redirection to the script.
 * - REMOTE_ADDR: IP address of the client.
 * - REMOTE_PORT: Port of the client.
 * - REQUEST_METHOD: HTTP method of the request.
 * - REQUEST_URI: Full request URI.
 * - SCRIPT_NAME: URI path of the CGI script until the script extension.
 * - SCRIPT_FILENAME: Actual file system path of the CGI script being executed.
 * - SERVER_ADDR: IP address of the server.
 * - SERVER_NAME: Name of the server.
 * - SERVER_PORT: Port of the server.
 * - SERVER_PROTOCOL: Protocol used by the server.
 * - SERVER_SOFTWARE: Software name and version of the server.
 * - SYSTEM_ROOT: Root directory of the server.
 * 
 * Pipes created:
 * - m_pipeIn: Pipe for input communication.
 * - m_pipeOut: Pipe for output communication.
 * 
 * If pipe creation fails, the request's HTTP status is set to StatusInternalServerError.
 * 
 * The constructor also calls setEnvp() to set the environment variables and setArgv() to set the
 * arguments for the execve system call.
 */
CGIHandler::CGIHandler(Connection& connection)
	: m_cgiPath(connection.location->cgiPath)
	, m_cgiExt(connection.location->cgiExt)
	, m_pipeIn()
	, m_pipeOut()
	, m_cgiPid(-1)
{

	/* ========= Set up environment for CGI script ========= */

	if (connection.m_request.headers.find("Content-Length") != connection.m_request.headers.end())
		m_env.push_back("CONTENT_LENGTH=" + connection.m_request.headers.at("Content-Length"));
	if (connection.m_request.headers.find("Content-Type") != connection.m_request.headers.end())
		m_env.push_back("CONTENT_TYPE=" + connection.m_request.headers.at("Content-Type"));
	m_env.push_back("GATEWAY_INTERFACE=CGI/1.1");
	const std::string pathInfo = extractPathInfo(connection.m_request.uri.path);
	m_env.push_back("PATH_INFO=" + pathInfo);
	m_env.push_back("PATH_TRANSLATED=" + connection.location->root + pathInfo);
	m_env.push_back("QUERY_STRING=" + connection.m_request.uri.query);
	m_env.push_back("REDIRECT_STATUS=200");
	m_env.push_back("REMOTE_ADDR=" + connection.m_clientSocket.host);
	m_env.push_back("REMOTE_PORT=" + connection.m_clientSocket.port);
	m_env.push_back("REQUEST_METHOD=" + webutils::methodToString(connection.m_request.method));
	m_env.push_back("REQUEST_URI=" + connection.m_request.uri.path + '?' + connection.m_request.uri.query);
	const std::string scriptPath = extractScriptPath(connection.m_request.uri.path);
	m_env.push_back("SCRIPT_NAME=" + scriptPath);
	m_env.push_back("SCRIPT_FILENAME=" + connection.location->root + scriptPath);
	m_env.push_back("SERVER_ADDR=" + connection.m_serverSocket.host);
	m_env.push_back("SERVER_NAME=" + connection.m_serverSocket.host);
	m_env.push_back("SERVER_PORT=" + connection.m_serverSocket.port);
	m_env.push_back("SERVER_PROTOCOL=HTTP/1.1");
	m_env.push_back("SERVER_SOFTWARE=Trihard/1.0.0");
	m_env.push_back("SYSTEM_ROOT=" + connection.location->root);

	/* ========= Create pipes for inter-process communication ========= */

	if (pipe(m_pipeIn) == -1) {
		LOG_ERROR << "pipe(): m_pipeIn: " << std::strerror(errno);
		connection.m_request.httpStatus = StatusInternalServerError;
	}
	if (pipe(m_pipeOut) == -1) {
		LOG_ERROR << "pipe(): pipeOut: " << std::strerror(errno);
		close(m_pipeIn[0]);
		close(m_pipeIn[1]);
		connection.m_request.httpStatus = StatusInternalServerError;
	}

	/* ========= Create input parameters for execve ========= */
	setEnvp();
	setArgv(connection.location->root + scriptPath);
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
 * @return const std::vector<std::string>& A constant reference to the environment variables.
 */
const std::vector<std::string> & CGIHandler::getEnv() const { return m_env; }

/* ====== MEMBER FUNCTIONS ====== */


/**
 * @brief Executes the CGI script for the given HTTP request.
 *
 * This function forks a new process to handle the CGI script execution. The child process
 * sets up the necessary pipes for communication, changes the working directory, and executes
 * the CGI script using execve. The parent process closes the unused ends of the pipes.
 *
 * @param request The HTTP request to be processed.
 * @param location An iterator pointing to the location configuration for the request.
 *
 * @note If the fork operation fails, the function logs an error, closes the pipes, sets the HTTP status
 *       to internal server error, and returns. If the child process encounters an error while setting up
 *       signals, changing the directory, or executing the CGI script, it writes an HTTP 500 error to stdout
 *       and exits.
 */
void CGIHandler::execute(HTTPRequest& request, std::vector<Location>::const_iterator& location)
{
	m_cgiPid = fork();
	if (m_cgiPid == -1) {
		LOG_ERROR << "fork(): " << std::strerror(errno);
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
			LOG_ERROR << "chdir(): " << std::strerror(errno);
			std::string error = "HTTP/1.1 500 Internal Server Error\r\n";
			write(STDOUT_FILENO, error.c_str(), error.size());
			std::exit(EXIT_FAILURE);
		}

		int ret = 0;
		ret = execve(m_argvp[0], m_argvp.data(), m_envp.data());
		LOG_ERROR << "execve(): " << std::strerror(errno);
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
 * This function converts the environment variables stored in the 
 * m_env vector to a format suitable for use with exec family of 
 * functions. It stores pointers to the C-style strings in the 
 * m_envp vector and appends a NULL pointer at the end to 
 * indicate the end of the environment list.
 */
void CGIHandler::setEnvp()
{
	for (std::vector<std::string>::iterator iter = m_env.begin(); iter != m_env.end(); iter++)
		m_envp.push_back(&(*iter).at(0));
	m_envp.push_back(NULL);
}


/**
 * @brief Sets up the argument vector (argv) for the CGI script execution.
 *
 * This function initializes the argument vector (argv) for the CGI script by
 * pushing the CGI path and the script filename into the m_argv vector. It then
 * converts these arguments into a format suitable for exec family functions by
 * storing pointers to the C-style strings in the m_argvp vector. A NULL pointer
 * is appended to m_argvp to mark the end of the arguments.
 *
 * @param scriptFilename The filename of the CGI script to be executed.
 */
void CGIHandler::setArgv(const std::string& scriptFilename)
{
	m_argv.push_back(m_cgiPath);
	m_argv.push_back(scriptFilename);
	for (std::vector<std::string>::iterator iter = m_argv.begin(); iter != m_argv.end(); iter++)
		m_argvp.push_back(&(*iter).at(0));
	m_argvp.push_back(NULL);
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
