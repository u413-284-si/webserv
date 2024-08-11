#include "CGIHandler.hpp"
#include "utilities.hpp"
#include <cstddef>
#include <sched.h>

/* ====== CONSTRUCTOR/DESTRUCTOR ====== */

CGIHandler::CGIHandler(std::string cgipath, std::string cgiExt)
	: m_cgiPath(cgipath)
	, m_cgiExt(cgiExt)
	, m_cgiPid(-1)
	, m_exitStatus(0)
{
}

/* ====== GETTER/SETTER FUNCTIONS ====== */

void CGIHandler::setCGIPid(pid_t cgiPid) { m_cgiPid = cgiPid; }

void CGIHandler::setCGIPath(const std::string& cgiPath) { m_cgiPath = cgiPath; }

const pid_t& CGIHandler::getCGIPid() const { return m_cgiPid; }

const std::string& CGIHandler::getCGIPath() const { return m_cgiPath; }

const std::map<std::string, std::string>& CGIHandler::getEnv() const { return m_env; }

/* ====== MEMBER FUNCTIONS ====== */

void CGIHandler::init(int clientFd, HTTPRequest& request, Location& location, unsigned short serverPort)
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
	m_env["REMOTE_ADDR"] = ; // IP address of the client -> getsocketname()
	if (request.headers.find("Authorization") != request.headers.end())
		m_env["AUTH_TYPE"] = request.headers["Authorization"].substr(0, request.headers["Authorization"].find(' '));
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
