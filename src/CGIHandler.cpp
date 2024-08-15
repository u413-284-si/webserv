#include "CGIHandler.hpp"
#include "Log.hpp"
#include "StatusCode.hpp"
#include "utilities.hpp"
#include <cstddef>
#include <sched.h>
#include <unistd.h>
#include <vector>

/* ====== CONSTRUCTOR/DESTRUCTOR ====== */

CGIHandler::CGIHandler(const std::string& cgipath, const std::string& cgiExt)
	: m_cgiPath(cgipath)
	, m_cgiExt(cgiExt)
{
}

/* ====== GETTER/SETTER FUNCTIONS ====== */

void CGIHandler::setCGIPath(const std::string& cgiPath) { m_cgiPath = cgiPath; }

const std::string& CGIHandler::getCGIPath() const { return m_cgiPath; }

const std::map<std::string, std::string>& CGIHandler::getEnv() const { return m_env; }

/* ====== MEMBER FUNCTIONS ====== */

statusCode CGIHandler::init(
	const int& clientSocket, HTTPRequest& request, const Location& location, const unsigned short& serverPort)
{
	m_env["REDIRECT_STATUS"]
		= "200"; // indicates successfull internal redirection to the script; required for old PHP scripts
	m_env["GATEWAY_INTERFACE"] = "CGI/1.1";
	m_env["SCRIPT_NAME"] = m_cgiPath; // URI path of the CGI script until the script extension
	m_env["SCRIPT_FILENAME"] = location.root + m_cgiPath; // actual file system path of the CGI script being executed
	m_env["REQUEST_METHOD"] = webutils::methodToString(request.method);
	m_env["PATH_INFO"] = extractPathInfo(request.uri.path);
	m_env["PATH_TRANSLATED"] = location.root + m_cgiPath;
	m_env["QUERY_STRING"] = request.uri.query;
	std::string clientIP;
	if (StatusOK != extractClientIP(clientSocket, clientIP))
		return StatusInternalServerError;
	m_env["REMOTE_ADDR"] = clientIP; // IP address of the client
	m_env["REQUEST_URI"] = request.uri.path + '?' + request.uri.query;
	std::stringstream strStream;
	strStream << serverPort;
	m_env["SERVER_PORT"] = strStream.str();
	m_env["SERVER_PROTOCOL"] = "HTTP/1.1";
	m_env["SERVER_SOFTWARE"] = "Trihard/1.0.0";
	if (request.headers.find("Content-Length") != request.headers.end())
		m_env["CONTENT_LENGTH"] = request.headers["Content-Length"];
	if (request.headers.find("Content-Type") != request.headers.end())
		m_env["CONTENT_TYPE"] = request.headers["Content-Type"];
	return StatusOK;
}

statusCode CGIHandler::execute()
{
	std::vector<std::string> envComposite;
	std::vector<std::string> argvAsStrings;
	std::vector<char*> envp;
	std::vector<char*> argv;
	char* endptr = NULL;

	setEnvp(envComposite, envp);

	argvAsStrings.push_back(m_env["SCRIPT_FILENAME"]);
	argvAsStrings.push_back(m_env["SCRIPT_NAME"]);
	for (std::vector<std::string>::iterator iter = argvAsStrings.begin(); iter != argvAsStrings.end(); iter++)
		argv.push_back(&(*iter).at(0));
	argv.push_back(endptr);

	int pipeIn[2]; // Pipe for passing input from server to CGI program
	int pipeOut[2]; // Pipe for passing output from CGI program to server
	if (pipe(pipeIn) == -1) {
		LOG_ERROR << "Error: pipe(): pipeIn";
		return StatusInternalServerError;
	}
	if (pipe(pipeOut) == -1) {
		LOG_ERROR << "Error: pipe(): pipeOut";
		close(pipeIn[0]);
		close(pipeIn[1]);
		return StatusInternalServerError;
	}

	pid_t cgiPid = fork();
	if (cgiPid == -1) {
		LOG_ERROR << "Error: fork()";
		close(pipeIn[0]);
		close(pipeIn[1]);
		close(pipeOut[0]);
		close(pipeOut[1]);
		return StatusInternalServerError;
	}
	if (cgiPid == 0) {
		dup2(pipeIn[0], STDIN_FILENO); // Replace child stdin with read end of input pipe
		dup2(pipeOut[1], STDOUT_FILENO); // Replace child stdout with write end of output pipe
		close(pipeIn[0]); // Can be closed as the read connection to server exists in stdin now
		close(pipeIn[1]);
		close(pipeOut[0]);
		close(pipeOut[1]); // Can be closed as the write connection to server exists in stdout now
		if (execve(argv[0], argv.data(), envp.data()) == -1) {
			std::string error = "Status: 500\r\n\r\n";
			write(STDOUT_FILENO, error.c_str(), error.size());
		}
	} else {
	}

	return StatusOK;
}

void CGIHandler::setEnvp(std::vector<std::string>& envComposite, std::vector<char*>& envp) const
{
	for (std::map<std::string, std::string>::const_iterator citer = m_env.begin(); citer != m_env.end(); citer++) {
		std::string tmp = citer->first + '=' + citer->second;
		envComposite.push_back(tmp);
	}
	for (std::vector<std::string>::iterator iter = envComposite.begin(); iter != envComposite.end(); iter++)
		envp.push_back(&(*iter).at(0)); // FIXME: remove reference to pointer value?
}

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

statusCode CGIHandler::extractClientIP(const int& clientSocket, std::string& clientIP)
{
	struct sockaddr_in clientAddress = {};
	socklen_t addrLen = sizeof(clientAddress);
	// NOLINTNEXTLINE: Ignore reinterpret_cast warning
	getsockname(clientSocket, reinterpret_cast<struct sockaddr*>(&clientAddress), &addrLen);

	// Convert the IP address to a string
	char dest[INET_ADDRSTRLEN];
	if (inet_ntop(AF_INET, &clientAddress.sin_addr, dest, INET_ADDRSTRLEN) == NULL) {
		LOG_ERROR << "Error: inet_ntop()";
		return StatusInternalServerError;
	}
	clientIP = dest;
	return StatusOK;
}
