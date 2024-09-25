#pragma once

/* ====== LIBRARIES ====== */

#include "ConfigFile.hpp"
#include "Connection.hpp"
#include "HTTPRequest.hpp"
#include "Log.hpp"
#include "Socket.hpp"
#include "StatusCode.hpp"
#include "signalHandler.hpp"
#include "utilities.hpp"

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

/* ====== DEFINITIONS ====== */

/* ====== CLASS DECLARATION ====== */

class CGIHandler {
public:
	explicit CGIHandler(Connection& connection);

	void execute(HTTPRequest& request, std::vector<Location>::const_iterator& location);

	// Getter functions
	int getPipeInWriteEnd() const;
	int getPipeOutReadEnd() const;
	const pid_t& getCGIPid() const;
	const std::string& getCGIPath() const;
	const std::string& getCGIExt() const;
	const std::map<std::string, std::string>& getEnv() const;

	void setEnvp(std::vector<std::string>& bufferEnv, std::vector<char*>& envp) const;
	void setArgv(std::vector<std::string>& bufferArgv, std::vector<char*>& argv);

private:
	std::string m_cgiPath; /**< Path to CGI interpreter */
	std::string m_cgiExt; /**< CGI script extension */
	std::map<std::string, std::string> m_env; /**< Environment variables for CGI script */
	int m_pipeIn[2]; /**< Pipe for passing input from server to CGI program */
	int m_pipeOut[2]; /**< Pipe for passing output from CGI program to server */
	pid_t m_cgiPid; /**< Process ID of the CGI process */

	// Helper functions
	std::string extractPathInfo(const std::string& path);
	std::string extractScriptPath(const std::string& path);
};

bool registerChildSignals();
