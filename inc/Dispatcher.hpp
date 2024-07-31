#pragma once

#include <cerrno>
#include <cstring>
#include <list>
#include <netdb.h>
#include <stdexcept>
#include <sys/epoll.h>
#include <unistd.h>
#include <vector>

#include "Log.hpp"

class IEndpoint;

class Dispatcher {
public:
	explicit Dispatcher(int timeout, size_t maxEvents = 1024);
	~Dispatcher();

	bool addEvent(int newfd, epoll_event* event, IEndpoint* endpoint);
	void removeEvent(int delfd) const;
	bool modifyEvent(int modfd, epoll_event* event) const;
	void handleEvents();
	bool addListeningEndpoint(const std::string& host, int backlog, const std::string& port);

private:
	int m_timeout;
	int m_epfd;
	std::vector<struct epoll_event> m_events;
	std::list<IEndpoint*> m_endpoints;
	std::string m_host;
	std::string m_port;

	bool createListeningEndpoint(const struct addrinfo* curr, int newFd);

	// Not copyable
	Dispatcher(const Dispatcher& other);
	Dispatcher& operator=(const Dispatcher& other);
};
