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

int CGIHandler::getPipeInWriteEnd() const { return m_pipeIn[1]; }

int CGIHandler::getPipeOutReadEnd() const { return m_pipeOut[0]; }

const pid_t& CGIHandler::getCGIPid() const { return m_cgiPid; }

/* ====== MEMBER FUNCTIONS ====== */

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
	m_env["SCRIPT_FILENAME"] = location->root + m_cgiPath; // actual file system path of the CGI script being executed
	m_env["SCRIPT_NAME"] = m_cgiPath; // URI path of the CGI script until the script extension
	m_env["SERVER_ADDR"] = serverSocket.host; // IP address of the server
	m_env["SERVER_NAME"] = serverSocket.host; // name of the server
	m_env["SERVER_PORT"] = serverSocket.port; // port of the server
	m_env["SERVER_PROTOCOL"] = "HTTP/1.1";
	m_env["SERVER_SOFTWARE"] = "Trihard/1.0.0";
	m_env["SYSTEM_ROOT"] = location->root;
}

void CGIHandler::execute(HTTPRequest& request)
{
	std::vector<std::string> envComposite;
	std::vector<std::string> argvAsStrings;
	std::vector<char*> envp;
	std::vector<char*> argv;
	char* endptr = NULL;

	setEnvp(envComposite, envp);

	argvAsStrings.push_back(m_env["SCRIPT_FILENAME"]);
	for (std::vector<std::string>::iterator iter = argvAsStrings.begin(); iter != argvAsStrings.end(); iter++)
		argv.push_back(&(*iter).at(0));
	argv.push_back(endptr);

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

void CGIHandler::setEnvp(std::vector<std::string>& envComposite, std::vector<char*>& envp) const
{
	for (std::map<std::string, std::string>::const_iterator citer = m_env.begin(); citer != m_env.end(); citer++) {
		std::string tmp = citer->first + '=' + citer->second;
		envComposite.push_back(tmp);
	}
	for (std::vector<std::string>::iterator iter = envComposite.begin(); iter != envComposite.end(); iter++)
		envp.push_back(&(*iter).at(0)); // FIXME: remove reference to pointer value?
	envp.push_back(NULL);
}

// HELPER FUNCTIONS

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
