#pragma once

/* ====== LIBRARIES ====== */
#include "Log.hpp"

#include <cstdio>
#include <cstring>
#include <sys/wait.h>
#include <unistd.h>

/* ====== CLASS DECLARATION ====== */

/**
 * @brief Wrapper class for process-related functions.
 *
 * This class provides wrappers for functions interacting with processes. It has function for the creation of pipes,
 * duplicating file descriptors, forking processes, changing directories, and executing processes. The following
 * functions are wrapped:
 * - pipe()
 * - dup2()
 * - chdir()
 * - fork()
 * - execve()
 * - read()
 * - write()
 * It can also be mocked for testing purposes.
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
	virtual ssize_t readProcess(int fileDescriptor, char* buf, size_t count) const;
	virtual ssize_t writeProcess(int fileDescriptor, const char* buf, size_t count) const;

private:
	ProcessOps(const ProcessOps&);
	ProcessOps& operator=(const ProcessOps&);
};
