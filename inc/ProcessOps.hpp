#pragma once

/* ====== LIBRARIES ====== */
#include "Log.hpp"

#include <cstring>
#include <unistd.h>
#include <sys/wait.h>

/* ====== CLASS DECLARATION ====== */

class ProcessOps {
public:
	ProcessOps();
	virtual ~ProcessOps();

	virtual int pipeProcess(int pipefd[2]);
	virtual int dup2Process(int oldfd, int newfd);
	virtual int chdirProcess(const char* path);
	virtual void forkProcess(pid_t& pid);
	virtual int execProcess(const char *pathname, char *const argv[], char *const envp[]);

private:
	ProcessOps(const ProcessOps&);
	ProcessOps& operator=(const ProcessOps&);
};
