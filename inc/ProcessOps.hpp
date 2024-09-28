#pragma once

/* ====== LIBRARIES ====== */
#include "Log.hpp"

#include <cstring>
#include <sys/wait.h>
#include <unistd.h>

/* ====== CLASS DECLARATION ====== */

/**
 * @brief Wrapper class for process-related functions.
 *
 * This class provides wrappers for functions regarding the creation of pipes, duplicating file descriptors, forking
 * processes, changing directories, and executing processes. It can also be mocked for testing purposes.
 */
class ProcessOps {
public:
	ProcessOps();
	virtual ~ProcessOps();

	virtual int pipeProcess(int pipefd[2]) const;
	virtual int dup2Process(int oldfd, int newfd) const;
	virtual int chdirProcess(const char* path) const;
	virtual int forkProcess(pid_t& pid) const;
	virtual int execProcess(const char* pathname, char* const argv[], char* const envp[]) const;

private:
	ProcessOps(const ProcessOps&);
	ProcessOps& operator=(const ProcessOps&);
};
