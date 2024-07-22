#pragma once

#include <sys/epoll.h>
#include <stdexcept>
#include <cerrno>
#include <string.h>
#include <vector>
#include <unistd.h>
#include "Log.hpp"

class Dispatcher {
	public:
		explicit Dispatcher(int timeout, size_t maxEvents = 1024);
		~Dispatcher();

		void addEvent(int newfd, epoll_event* event) const;
		void removeEvent(int delfd) const;
		void modifyEvent(int modfd, epoll_event* event) const;
		int wait();

	private:
		int m_timeout;
		int m_epfd;
		std::vector<struct epoll_event> m_events;

		// Not copyable
		Dispatcher(const Dispatcher& other);
		Dispatcher& operator=(const Dispatcher& other);
};
