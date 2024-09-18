#pragma once

/* ====== LIBRARIES ====== */

#include "ConfigFile.hpp"
#include "HTTPRequest.hpp"
#include "Log.hpp"
#include "Socket.hpp"
#include "StatusCode.hpp"
#include "utilities.hpp"

#include <arpa/inet.h>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <sched.h>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

/* ====== DEFINITIONS ====== */

#define BUFFER_SIZE 50000000 // 50 Megabyte

/* ====== CLASS DECLARATION ====== */

class CGIHandler {
public:
	explicit CGIHandler(const std::string& cgiPath, const std::string& cgiExt);

	void init(const Socket& clientSocket, const Socket& serverSocket, const HTTPRequest& request,
		const std::vector<Location>::const_iterator& location);
	void execute(HTTPRequest& request);

	// Getter functions
	int getPipeInWriteEnd() const;
	int getPipeOutReadEnd() const;
	const pid_t& getCGIPid() const;

	void setEnvp(std::vector<std::string>& bufferEnv, std::vector<char*>& envp) const;
	void setArgv(std::vector<std::string>& bufferArgv, std::vector<char*>& argv);

private:
	std::string m_cgiPath; /**< URL until CGI script extension */
	std::string m_cgiExt; /**< CGI script extension */
	std::map<std::string, std::string> m_env; /**< Environment variables for CGI script */
	int m_pipeIn[2]; /**< Pipe for passing input from server to CGI program */
	int m_pipeOut[2]; /**< Pipe for passing output from CGI program to server */
	pid_t m_cgiPid; /**< Process ID of the CGI process */

	// Helper functions
	std::string extractPathInfo(const std::string& path);
	std::string extractScriptPath(const std::string& path);
};
