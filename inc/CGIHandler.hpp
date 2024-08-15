#pragma once

/* ====== LIBRARIES ====== */

#include "ConfigFile.hpp"
#include "HTTPRequest.hpp"
#include "StatusCode.hpp"
#include "utilities.hpp"

#include <arpa/inet.h>
#include <cstdlib>
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <sched.h>
#include <string>
#include <vector>

/* ====== DEFINITIONS ====== */

/* ====== CLASS DECLARATION ====== */

class CGIHandler {
public:
	int pipeIn[2]; // pipe for passing input from server to CGI program
	int pipeOut[2]; // pipe for passing output from CGI program to server

	explicit CGIHandler(const std::string& cgipath, const std::string& cgiExt);

	statusCode init(const int& clientSocket, HTTPRequest& request, const Location& location, const unsigned short& serverPort);
	statusCode execute();

private:
	std::string m_cgiPath; // URL until CGI script extension
	std::string m_cgiExt;
	std::map<std::string, std::string> m_env;

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

	void setEnvp(std::vector<std::string>& envComposite, std::vector<char*>& envp) const;

	// Helper functions
	std::string extractPathInfo(const std::string& path);
	static statusCode extractClientIP(const int& clientSocket, std::string& clientIP);
};
