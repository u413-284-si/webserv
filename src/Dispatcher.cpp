#include "Dispatcher.hpp"
#include "ListeningEndpoint.hpp"

Dispatcher::Dispatcher(const int timeout, const size_t maxEvents)
	: m_timeout(timeout)
	, m_epfd(epoll_create(1))
	, m_events(maxEvents)
{
	if (m_epfd == -1) {
		LOG_ERROR << "epoll_create: " << strerror(errno) << '\n';
		throw std::runtime_error("epoll_create:" + std::string(strerror(errno)));
	}
	m_host.resize(NI_MAXHOST);
	m_port.resize(NI_MAXSERV);
}

Dispatcher::~Dispatcher() { close(m_epfd); }

Dispatcher::Dispatcher(const Dispatcher& other)
	: m_timeout(other.m_timeout)
	, m_epfd(other.m_epfd)
	, m_events(other.m_events)
{
}

Dispatcher& Dispatcher::operator=(const Dispatcher& other)
{
	if (this != &other) {
		m_epfd = other.m_epfd;
		m_events = other.m_events;
	}
	return *this;
}

bool Dispatcher::addEvent(const int newfd, epoll_event* event, IEndpoint* endpoint)
{
	if (epoll_ctl(m_epfd, EPOLL_CTL_ADD, newfd, event) == -1) {
		LOG_ERROR << "epoll_ctl: EPOLL_CTL_ADD: " << strerror(errno) << '\n';
		return false;
	}
	LOG_DEBUG << "epoll_ctl: Added new fd: " << newfd;

	m_endpoints.push_back(endpoint);
	LOG_DEBUG << "Added new endpoint";

	return true;
}

void Dispatcher::removeEvent(const int delfd, IEndpoint* endpoint)
{
	if (epoll_ctl(m_epfd, EPOLL_CTL_DEL, delfd, NULL) == -1) {
		LOG_ERROR << "epoll_ctl: EPOLL_CTL_DEL: " << strerror(errno) << '\n';
	}
	LOG_DEBUG << "epoll_ctl: Removed fd: " << delfd << '\n';

	m_endpoints.remove(endpoint);
	delete endpoint;
	LOG_DEBUG << "Removed endpoint";
}

bool Dispatcher::modifyEvent(const int modfd, epoll_event* event) const
{
	if (epoll_ctl(m_epfd, EPOLL_CTL_MOD, modfd, event) == -1) {
		LOG_ERROR << "epoll_ctl: EPOLL_CTL_MOD: " << strerror(errno) << '\n';
		return false;
	}
	LOG_DEBUG << "epoll_ctl: Modified fd: " << modfd << '\n';
	return true;
}

void Dispatcher::handleEvents()
{
	while (true) {
		const int nfds = epoll_wait(m_epfd, &m_events[0], static_cast<int>(m_events.size()), m_timeout);
		if (nfds == -1) {
			LOG_ERROR << "epoll_wait: " << strerror(errno);
			throw std::runtime_error("epoll_wait:" + std::string(strerror(errno)));
		}
		if (nfds == 0)
			LOG_DEBUG << "epoll_wait: Timeout";
		else
			LOG_DEBUG << "epoll_wait: " << nfds << " events";

		for (std::vector<struct epoll_event>::iterator iter = m_events.begin(); iter != m_events.begin() + nfds;
			 ++iter) {
			uint32_t eventMask = iter->events;
			if ((eventMask & EPOLLERR) != 0) {
				LOG_DEBUG << "epoll_wait: EPOLLERR";
				eventMask = EPOLLOUT;
			} else if ((eventMask & EPOLLHUP) != 0) {
				LOG_DEBUG << "epoll_wait: EPOLLHUP";
				eventMask = EPOLLOUT;
			}
			static_cast<IEndpoint*>(iter->data.ptr)->handleEvent(*this, eventMask);
		}
	}
}

bool Dispatcher::addListeningEndpoint(const std::string& host, const int backlog, const std::string& port)
{
	const char* node = NULL;

	if (!host.empty() || host != "*")
		node = host.c_str();

	struct addrinfo hints = {};
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 0;
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;

	struct addrinfo* list = NULL;
	const int result = getaddrinfo(node, port.c_str(), &hints, &list);
	if (result != 0) {
		LOG_ERROR << "getaddrinfo(): " << gai_strerror(result) << '\n';
		return false;
	}
	size_t successfulSock = 0;
	for (struct addrinfo* curr = list; curr != NULL; curr = curr->ai_next) {
		LOG_DEBUG << "Trying to bind " << successfulSock + 1 << ". time";

		const int newFd = socket(curr->ai_family, curr->ai_socktype | SOCK_NONBLOCK, curr->ai_protocol);
		if (newFd == -1) {
			LOG_DEBUG << "socket(): " << strerror(errno);
			continue;
		}

		if (bind(newFd, curr->ai_addr, curr->ai_addrlen) == -1) {
			close(newFd);
			LOG_DEBUG << "bind(): " << strerror(errno);
			continue;
		}

		int reuse = 1;
		if (setsockopt(newFd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) == -1) {
			close(newFd);
			LOG_DEBUG << "setsockopt(): " << strerror(errno);
			continue;
		}

		if (listen(newFd, backlog) == -1) {
			close(newFd);
			LOG_DEBUG << "listen(): " << strerror(errno);
			continue;
		}

		if (!createListeningEndpoint(curr, newFd)) {
			continue;
		}

		++successfulSock;
	}
	// We don't need the list anymore
	freeaddrinfo(list);

	if (successfulSock == 0) {
		LOG_ERROR << "Cannot bind to a valid socket.\n";
		return false;
	}

	return true;
}

bool Dispatcher::createListeningEndpoint(const struct addrinfo* curr, const int newFd)
{
	const int ret = getnameinfo(curr->ai_addr, curr->ai_addrlen, &m_host[0], NI_MAXHOST, &m_port[0], NI_MAXSERV,
		NI_NUMERICHOST | NI_NUMERICSERV);

	if (ret != 0) {
		close(newFd);
		LOG_ERROR << "getnameinfo(): " << gai_strerror(ret) << '\n';
		return false;
	}

	const Socket newSock = { newFd, m_host, m_port };

	IEndpoint* endpoint = new ListeningEndpoint(newSock);

	epoll_event event = {};
	event.events = EPOLLIN | EPOLLET;
	event.data.ptr = static_cast<void*>(endpoint);
	if (!addEvent(newFd, &event, endpoint)) {
		close(newFd);
		delete endpoint;
		return false;
	}
	LOG_INFO << "Created listening endpoint: " << newSock;
	return true;
}
