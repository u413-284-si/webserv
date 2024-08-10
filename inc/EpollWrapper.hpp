#pragma once

#include "Log.hpp"

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <sys/epoll.h>
#include <unistd.h>
#include <vector>

/**
 * @brief Wrapper class for epoll instance.
 *
 * This class is a wrapper for the epoll instance. It is responsible for the creation and destruction
 * of the epoll instance. It also provides a function to wait for events on the epoll instance and functions to add,
 * modify and remove file descriptors from the epoll instance.
 * It can also be mocked for testing purposes.
 */
class EpollWrapper {

private:
	static const int s_epollMaxEvents = 10; /**< Default maximum number of events for epoll instance */
	static const int s_epollTimeout = 1000; /**< Default timeout for epoll instance in miliseconds */

public:
	explicit EpollWrapper(size_t maxEvents = s_epollMaxEvents, int epollTimeout = s_epollTimeout);
	virtual ~EpollWrapper();

	virtual int waitForEvents();
	virtual std::vector<struct epoll_event>::const_iterator eventsBegin() const;
	virtual bool addEvent(int newfd, epoll_event& event) const;
	virtual void removeEvent(int delfd) const;
	virtual bool modifyEvent(int modfd, epoll_event& event) const;

private:
	int m_epfd; /**< FD of epoll instance */
	std::vector<struct epoll_event> m_events; /**< Holds epoll events */
	int m_timeout; /**< Timeout for epoll instance in milliseconds */

	EpollWrapper(const EpollWrapper&);
	EpollWrapper& operator=(const EpollWrapper&);
};
