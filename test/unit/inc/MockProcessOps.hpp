#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ProcessOps.hpp"

/**
 * @brief Mock class for ProcessOps.
 *
 */
class MockProcessOps : public ProcessOps {
public:
	MOCK_METHOD(int, pipeProcess, (int pipefd[2]), (const, override));
	MOCK_METHOD(int, dup2Process, (int oldfd, int newfd), (const, override));
	MOCK_METHOD(int, chdirProcess, (const char* path), (const, override));
	MOCK_METHOD(int, forkProcess, (pid_t & pid), (const, override));
	MOCK_METHOD(int, execProcess, (const char* pathname, char* const argv[], char* const envp[]), (const, override));
	MOCK_METHOD(ssize_t, readProcess, (int fileDescriptor, char* buf, size_t count), (const, override));
	MOCK_METHOD(ssize_t, writeProcess, (int fileDescriptor, const char* buf, size_t count), (const, override));
	MOCK_METHOD(pid_t, waitForProcess, (pid_t pid, int* wstatus, int options), (const, override));
};
