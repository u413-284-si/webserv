#include "CGIHandler.hpp"
#include "ConfigFile.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "Log.hpp"
#include "StatusCode.hpp"
#include "utilities.hpp"
#include <cstddef>
#include <cstdlib>
#include <sched.h>
#include <unistd.h>
#include <vector>

/* ====== CONSTRUCTOR/DESTRUCTOR ====== */

/**
 * @brief Constructs a CGIHandler object with the specified CGI path and extension.
 *
 * This constructor initializes the CGIHandler with the provided CGI path and extension.
 * It also initializes the pipe file descriptors and the CGI process ID.
 *
 * @param cgiPath The path to the CGI interpreter.
 * @param cgiExt The file extension associated with the CGI script.
 */
CGIHandler::CGIHandler(const std::string& cgiPath, const std::string& cgiExt)
	: m_cgiPath(cgiPath)
	, m_cgiExt(cgiExt)
	, m_pipeIn()
	, m_pipeOut()
	, m_cgiPid(-1)
{
	m_pipeIn[0] = -1;
	m_pipeIn[1] = -1;
	m_pipeOut[0] = -1;
	m_pipeOut[1] = -1;
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

/* ====== MEMBER FUNCTIONS ====== */

/**
 * @brief Initializes the CGI environment variables for the CGIHandler.
 *
 * This function sets up the necessary environment variables required for
 * executing a CGI script based on the provided client socket, server socket,
 * HTTP request, and location configuration.
 *
 * @param clientSocket The socket representing the client connection.
 * @param serverSocket The socket representing the server connection.
 * @param request The HTTP request containing headers and URI information.
 * @param location An iterator to the location configuration in the server settings.
 */
void CGIHandler::init(const Socket& clientSocket, const Socket& serverSocket, const HTTPRequest& request,
	const std::vector<Location>::const_iterator& location)
{
	if (request.headers.find("Content-Length") != request.headers.end())
		m_env["CONTENT_LENGTH"] = request.headers.at("Content-Length");
	if (request.headers.find("Content-Type") != request.headers.end())
		m_env["CONTENT_TYPE"] = request.headers.at("Content-Type");
	m_env["GATEWAY_INTERFACE"] = "CGI/1.1";
	m_env["PATH_INFO"] = extractPathInfo(request.uri.path); // additional path information following the script name
	m_env["PATH_TRANSLATED"] = location->root + m_env["PATH_INFO"]; // actual file system path of the PATH_INFO resource
	m_env["QUERY_STRING"] = request.uri.query;
	m_env["REDIRECT_STATUS"]
		= "200"; // indicates successfull internal redirection to the script; required for old PHP scripts
	m_env["REMOTE_ADDR"] = clientSocket.host; // IP address of the client
	m_env["REMOTE_PORT"] = clientSocket.port; // port of the client
	m_env["REQUEST_METHOD"] = webutils::methodToString(request.method);
	m_env["REQUEST_URI"] = request.uri.path + '?' + request.uri.query;
	m_env["SCRIPT_NAME"] = extractScriptPath(request.uri.path); // URI path of the CGI script until the script extension
	m_env["SCRIPT_FILENAME"]
		= location->root + m_env["SCRIPT_NAME"]; // actual file system path of the CGI script being executed
	m_env["SERVER_ADDR"] = serverSocket.host; // IP address of the server
	m_env["SERVER_NAME"] = serverSocket.host; // name of the server
	m_env["SERVER_PORT"] = serverSocket.port; // port of the server
	m_env["SERVER_PROTOCOL"] = "HTTP/1.1";
	m_env["SERVER_SOFTWARE"] = "Trihard/1.0.0";
	m_env["SYSTEM_ROOT"] = location->root;
}

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
void CGIHandler::execute(HTTPRequest& request)
{
	std::vector<std::string> bufferEnv;
	std::vector<std::string> bufferArgv;
	std::vector<char*> envp;
	std::vector<char*> argv;

	setEnvp(bufferEnv, envp);
	setArgv(bufferArgv, argv);

	if (pipe(m_pipeIn) == -1) {
		LOG_ERROR << "Error: pipe(): m_pipeIn: " + std::string(std::strerror(errno));
		request.httpStatus = StatusInternalServerError;
		return;
	}
	if (pipe(m_pipeOut) == -1) {
		LOG_ERROR << "Error: pipe(): pipeOut: " + std::string(std::strerror(errno));
		close(m_pipeIn[0]);
		close(m_pipeIn[1]);
		request.httpStatus = StatusInternalServerError;
		return;
	}

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
		dup2(m_pipeIn[0], STDIN_FILENO); // Replace child stdin with read end of input pipe
		dup2(m_pipeOut[1], STDOUT_FILENO); // Replace child stdout with write end of output pipe
		close(m_pipeIn[0]); // Can be closed as the read connection to server exists in stdin now
		close(m_pipeIn[1]);
		close(m_pipeOut[0]);
		close(m_pipeOut[1]); // Can be closed as the write connection to server exists in stdout now
		int ret = 0;
		ret = execve(argv[0], argv.data(), envp.data());
		LOG_ERROR << "Error: execve(): " + std::string(std::strerror(errno));
		if (ret == -1) {
			std::string error = "HTTP/1.1 500 Internal Server Error\r\n";
			write(STDOUT_FILENO, error.c_str(), error.size());
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
