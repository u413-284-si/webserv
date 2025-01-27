#pragma once

/* ====== LIBRARIES ====== */

#include "ConfigFile.hpp"
#include "Connection.hpp"
#include "FileSystemOps.hpp"
#include "HTTPRequest.hpp"
#include "Log.hpp"
#include "ProcessOps.hpp"
#include "Socket.hpp"
#include "StatusCode.hpp"
#include "TargetResourceHandler.hpp"
#include "signalHandler.hpp"
#include "utilities.hpp"

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

/* ====== DEFINITIONS ====== */

/* ====== CLASS DECLARATION ====== */

class CGIHandler {
public:
	explicit CGIHandler(Connection& connection, const ProcessOps& processOps, const FileSystemOps& fileSystemOps);

	void execute(
		int epollFd, const std::map<int, Connection>& connections, const std::map<int, Connection*>& cgiConnections, const std::map<int, Socket>& virtualServers);

	// Getter functions
	const std::string& getCGIPath() const;
	const std::string& getCGIExt() const;
	const std::vector<std::string>& getEnv() const;
	const std::vector<std::string>& getArgv() const;

private:
	const ProcessOps& m_processOps; /**< Process operations object */
	const FileSystemOps& m_fileSystemOps; /**< File system operations object */
	std::string m_cgiPath; /**< Path to CGI interpreter */
	std::string m_cgiExt; /**< CGI script extension */
	std::string m_cgiScriptPath; /**< Path to the CGI script */
	std::vector<std::string> m_env; /**< Environment variables for CGI script */
	std::vector<std::string> m_argv; /**< Arguments for CGI script */
	std::vector<char*> m_envp; /**< Pointers to environment variables for CGI script */
	std::vector<char*> m_argvp; /**< Pointers to arguments for CGI script */
	int m_pipeIn[2]; /**< Pipe for passing input from server to CGI program */
	int m_pipeOut[2]; /**< Pipe for passing output from CGI program to server */
	int& m_pipeToCGIWriteEnd; /**< Write end of the pipe to the CGI process */
	int& m_pipeFromCGIReadEnd; /**< Read end of the pipe from the CGI process */
	pid_t& m_cgiPid; /**< Process ID of the CGI process */
	HTTPRequest& m_request; /**< HTTP request object */
	std::vector<Location>::const_iterator m_location; /**< Const iterator to Location object */
	std::vector<ConfigServer>::const_iterator m_serverConfig; /**< Const iterator to ConfigServer object */

	void setEnvp();
	void setArgv();

	// Helper functions
	std::string extractPathInfo(const std::string& path);
	std::string extractScriptPath(const std::string& path);
	std::string extractPreScriptPath(const std::string& path);
	std::string mapPathInfoToFileSystem(const std::string& path);
	void closePipes();
	void closeAllFds(int epollFd, const std::map<int, Connection>& connections, const std::map<int, Connection*>& cgiConnections, const std::map<int, Socket>& virtualServers);
};

bool registerChildSignals();
