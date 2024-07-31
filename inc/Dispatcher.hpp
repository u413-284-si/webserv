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
private:
	static const int s_epollTimeout = 1000;
	static const int s_epollMaxEvents = 10;
	static const time_t s_clientTimeout = 60;

public:
	explicit Dispatcher(int epollTimeout = s_epollTimeout, size_t maxEvents = s_epollMaxEvents);
	~Dispatcher();

	bool addEvent(int newfd, epoll_event* event, IEndpoint* endpoint);
	void removeEvent(int delfd) const;
	bool modifyEvent(int modfd, epoll_event* event) const;
	void handleEvents();
	bool addListeningEndpoint(const std::string& host, int backlog, const std::string& port);

private:
	int m_epollTimeout;
	int m_epfd;
	std::vector<struct epoll_event> m_epollEvents;
	std::list<IEndpoint*> m_endpoints;
	std::string m_host;
	std::string m_port;
	time_t m_clientTimeout;

	bool createListeningEndpoint(const struct addrinfo* curr, int newFd);
	void handleTimeout();
	void removeInactiveEndpoints();

	// Not copyable
	Dispatcher(const Dispatcher& other);
	Dispatcher& operator=(const Dispatcher& other);
};
