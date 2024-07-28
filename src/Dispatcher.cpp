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
		else if (nfds == 0)
			LOG_DEBUG << "epoll_wait: Timeout";
		else
			LOG_DEBUG << "epoll_wait: " << nfds << " events";

		for (std::vector<struct epoll_event>::iterator iter = m_events.begin(); iter != m_events.begin() + nfds; ++iter) {
			uint32_t eventMask = iter->events;
			if ((eventMask & EPOLLERR) != 0) {
				LOG_DEBUG << "epoll_wait: EPOLLERR";
				eventMask = EPOLLOUT;
			}
			else if ((eventMask & EPOLLHUP) != 0) {
				LOG_DEBUG << "epoll_wait: EPOLLHUP";
				eventMask = EPOLLOUT;
			}
			static_cast<IEndpoint *>(iter->data.ptr)->handleEvent(*this, eventMask);
		}
	}
}

bool Dispatcher::initServer(const std::string& host, const int backlog, const std::string& port)
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

	struct addrinfo *list = NULL;
	const int result = getaddrinfo(node, port.c_str(), &hints, &list);
	if (result != 0) {
		LOG_ERROR << "initServer(): " << gai_strerror(result) << '\n';
		return false;
	}
	int newFd = -1;
	for (struct addrinfo *curr = list; curr != NULL; curr = curr->ai_next) {

		newFd = socket(curr->ai_family, curr->ai_socktype | SOCK_NONBLOCK, curr->ai_protocol);
		if (newFd == -1)
			continue;

		if (bind(newFd, curr->ai_addr, curr->ai_addrlen) == -1) {
			close(newFd);
			newFd = -1;
			continue;
		}

		int reuse = 1;
		if (setsockopt(newFd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) == -1) {
			close(newFd);
			newFd = -1;
			continue;
		}

		if (listen(newFd, backlog) == -1) {
			close(newFd);
			newFd = -1;
			continue;
		}

		char bufHost[NI_MAXHOST];
	char bufPort[NI_MAXSERV];
	const int ret = getnameinfo(list->ai_addr, list->ai_addrlen, bufHost, NI_MAXHOST, bufPort, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);
	// We don't need the list anymore

	if (ret != 0) {
		LOG_ERROR << "initServer(): " << gai_strerror(ret) << '\n';
		close(newFd);
		return false;
	}

	const Socket newSock = { newFd, bufHost, bufPort };
	IEndpoint* endpoint = new ListeningEndpoint(newSock);
	epoll_event event = {};
	event.events = EPOLLIN | EPOLLET;
	event.data.ptr = static_cast<void *>(endpoint);
	if (!addEvent(newFd, &event, endpoint)) {
		close(newFd);
		delete endpoint;
		return false;
	}
	LOG_INFO << "Added server endpoint: " << newSock.host << ':' << newSock.port;
	}
	if (newFd == -1) {
		LOG_ERROR << "initServer(): Cannot bind to a valid socket.\n";
		return false;
	}


	return true;
}
