#pragma once

#include <sys/epoll.h>
#include <stdexcept>
#include <cerrno>
#include <string.h>
#include <vector>
#include <list>
#include <unistd.h>
#include <netdb.h>

#include "Log.hpp"
#include "ServerEndpoint.hpp"

class Dispatcher {
	public:
		explicit Dispatcher(int timeout, size_t maxEvents = 1024);
		~Dispatcher();

		bool addEvent(int newfd, epoll_event* event) const;
		bool removeEvent(int delfd) const;
		bool modifyEvent(int modfd, epoll_event* event) const;
		void handleEvents();
		bool initServer(const std::string& host, int backlog, const std::string& port);

	private:
		int m_timeout;
		int m_epfd;
		std::vector<struct epoll_event> m_events;
		std::list<IEndpoint*> m_endpoints;

		// Not copyable
		Dispatcher(const Dispatcher& other);
		Dispatcher& operator=(const Dispatcher& other);
};
