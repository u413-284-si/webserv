#include "ProcessOps.hpp"

/* ====== CONSTRUCTOR/DESTRUCTOR ====== */

ProcessOps::ProcessOps() { }
ProcessOps::~ProcessOps() { }

/* ====== MEMBER FUNCTIONS ====== */


/**
 * @brief Creates a pipe and stores the file descriptors in the provided array.
 * 
 * This function wraps the `pipe` system call to create a unidirectional data channel 
 * that can be used for inter-process communication. The file descriptors for the read 
 * and write ends of the pipe are stored in the array provided by the caller.
 * 
 * @param pipefd An array of two integers where the file descriptors for the read and 
 * write ends of the pipe will be stored.
 * @return int Returns 0 on success, or -1 if an error occurs. In case of an error, 
 * an error message is logged and the global variable `errno` is set to indicate the error.
 */
int ProcessOps::pipeProcess(int pipefd[2]) const
{
	if (pipe(pipefd) == -1) {
		LOG_ERROR << "pipe(): " << std::strerror(errno);
		return -1;
	}
	return 0;
}

/**
 * @brief Duplicates a file descriptor to a specified value.
 *
 * This function duplicates the file descriptor `oldfd` to `newfd` using the `dup2` system call.
 * If the duplication fails, it logs an error message and returns -1.
 *
 * @param oldfd The file descriptor to duplicate.
 * @param newfd The file descriptor to duplicate to.
 * @return int Returns 0 on success, or -1 on failure.
 */
int ProcessOps::dup2Process(int oldfd, int newfd) const
{
	if (dup2(oldfd, newfd) == -1) {
		LOG_ERROR << "dup2(): " << std::strerror(errno);
		return -1;
	}
	return 0;
}

/**
 * @brief Changes the current working directory.
 *
 * This function changes the current working directory to the directory specified in `path`
 * using the `chdir` system call. If the operation fails, it logs an error message and returns -1.
 *
 * @param path The path to the directory to change to.
 * @return int Returns 0 on success, or -1 on failure.
 */
int ProcessOps::chdirProcess(const char* path) const
{
	if (chdir(path) == -1) {
		LOG_ERROR << "chdir(): " << std::strerror(errno);
		return -1;
	}
	return 0;
}

/**
 * @brief Forks the current process.
 *
 * This function attempts to create a new process by calling the fork() system call.
 * The process ID of the child process is stored in the provided pid reference.
 * If the fork() call fails, an error message is logged.
 *
 * @param pid Reference to a pid_t variable where the process ID of the child process will be stored.
 * @return int Returns 0 on success, or -1 on failure.
 */
int ProcessOps::forkProcess(pid_t& pid) const
{
	pid = fork();
	if (pid == -1) {
		LOG_ERROR << "fork(): " << std::strerror(errno);
		return -1;
	}
	return 0;
}

/**
 * @brief Executes a new process image specified by the given pathname.
 *
 * This function replaces the current process image with a new process image
 * specified by the pathname. The new process image will be executed with the
 * provided arguments and environment variables.
 *
 * @param pathname The path to the executable file.
 * @param argv A null-terminated array of character pointers to arguments.
 *             The first element should be the name of the program.
 * @param envp A null-terminated array of character pointers to environment
 *             variables.
 * @return Returns 0 on success. On failure, returns -1 and logs an error
 *         message with the reason for the failure.
 */
int ProcessOps::execProcess(const char *pathname, char *const argv[], char *const envp[]) const
{
	if (execve(pathname, argv, envp) == -1) {
		LOG_ERROR << "execve(): " << std::strerror(errno);
		return -1;
	}
	return 0;
}

/**
 * @brief Reads data from a file descriptor into a buffer.
 *
 * This function attempts to read up to `count` bytes from the file descriptor
 * specified by `fileDescriptor` into the buffer pointed to by `buf`.
 *
 * @param fileDescriptor The file descriptor from which to read.
 * @param buf A pointer to the buffer where the read data should be stored.
 * @param count The maximum number of bytes to read.
 * @return The number of bytes read on success, or -1 on error.
 *         If an error occurs, an error message is logged.
 */
ssize_t ProcessOps::readProcess(int fileDescriptor, char *buf, size_t count) const
{
    ssize_t bytesRead = read(fileDescriptor, buf, count);
    if (bytesRead == -1) {
        LOG_ERROR << "read(): " << std::strerror(errno);
        return -1;
    }
    return bytesRead;
}

/**
 * @brief Writes data from a buffer to a file descriptor.
 *
 * This function attempts to write up to `count` bytes from the buffer pointed to by `buf`
 * to the file descriptor specified by `fileDescriptor`.
 *
 * @param fileDescriptor The file descriptor to which to write.
 * @param buf A pointer to the buffer containing the data to write.
 * @param count The number of bytes to write.
 * @return The number of bytes written on success, or -1 on error.
 *         If an error occurs, an error message is logged.
 */
ssize_t ProcessOps::writeProcess(int fileDescriptor, const char *buf, size_t count) const
{
    ssize_t bytesWritten = write(fileDescriptor, buf, count);
    if (bytesWritten == -1) {
        LOG_ERROR << "write(): " << std::strerror(errno);
        return -1;
    }
    return bytesWritten;
}
