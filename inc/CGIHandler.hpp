#pragma once

/* ====== LIBRARIES ====== */

#include "ConfigFile.hpp"
#include "HTTPRequest.hpp"
#include "utilities.hpp"
#include <cstdlib>
#include <iostream>
#include <map>
#include <sched.h>
#include <string>
#include <vector>

/* ====== DEFINITIONS ====== */

/* ====== CLASS DECLARATION ====== */

class CGIHandler {
public:
	int pipeIn[2]; // pipe for passing input from server to CGI program
	int pipeOut[2]; // pipe for passing output from CGI program to server

	explicit CGIHandler(std::string cgipath, std::string cgiExt);

	void init(int clientFd, HTTPRequest& request, Location& location, unsigned short serverPort);

private:
	std::string m_cgiPath; // URL until CGI script extension
	std::string m_cgiExt;
	std::map<std::string, std::string> m_env;
	std::vector<char*> m_envp;
	std::vector<char*> m_argv;

	pid_t m_cgiPid;
	int m_exitStatus;

	CGIHandler(const CGIHandler& ref);
	CGIHandler& operator=(const CGIHandler& rhs);
	~CGIHandler();

	// Getter/Setter functions
	void setCGIPid(pid_t cgiPid);
	void setCGIPath(const std::string& cgiPath);

	const pid_t& getCGIPid() const;
	const std::string& getCGIPath() const;
	const std::map<std::string, std::string>& getEnv() const;

	std::string extractPathInfo(const std::string& path);
};
