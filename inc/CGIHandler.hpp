#pragma once

/* ====== LIBRARIES ====== */

#include "ConfigFile.hpp"
#include "HTTPRequest.hpp"
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
	explicit CGIHandler(const std::string& cgipath, const std::string& cgiExt);

	statusCode init(
		const int& clientSocket, HTTPRequest& request, const Location& location, const unsigned short& serverPort);
	statusCode execute(HTTPRequest& request, std::string& newBody);

private:
    static const std::size_t s_cgiBodyBufferSize = 32000; /**< Default output buffer size for CGI body in Bytes */
    
	std::string m_cgiPath; /**< URL until CGI script extension */
	std::string m_cgiExt;
	std::map<std::string, std::string> m_env;

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
	static statusCode sendDataToCGIProcess(int pipeInWriteEnd, HTTPRequest& request);
	static statusCode receiveDataFromCGIProcess(int pipeOutReadEnd, pid_t& cgiPid, std::string& newBody);

	// Helper functions
	std::string extractPathInfo(const std::string& path);
	static statusCode extractClientIP(const int& clientSocket, std::string& clientIP);
};
