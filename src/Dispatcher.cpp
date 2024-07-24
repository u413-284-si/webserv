#include "Dispatcher.hpp"
#include "ServerEndpoint.hpp"

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

bool Dispatcher::removeEvent(const int delfd) const
{
	if (epoll_ctl(m_epfd, EPOLL_CTL_DEL, delfd, NULL) == -1) {
		LOG_ERROR << "epoll_ctl: EPOLL_CTL_DEL: " << strerror(errno) << '\n';
		return false;
	}
	LOG_DEBUG << "epoll_ctl: Removed fd: " << delfd << '\n';
	return true;
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
		if (nfds == -1)
			LOG_ERROR << "epoll_wait: " << strerror(errno) << '\n';
		else if (nfds == 0)
			LOG_DEBUG << "epoll_wait: Timeout\n";
		else
			LOG_DEBUG << "epoll_wait: " << nfds << " events\n";

		for (std::vector<struct epoll_event>::iterator iter = m_events.begin(); iter != m_events.begin() + nfds; ++iter) {
			if (iter->events & EPOLLERR || iter->events & EPOLLHUP) {
				LOG_ERROR << "epoll_wait: EPOLLERR or EPOLLHUP\n";
				close(iter->data.fd);
				continue;
			}
			if (iter->events & EPOLLIN) {
				static_cast<IEndpoint *>(iter->data.ptr)->handleEvent(*this);
				LOG_DEBUG << "epoll_wait: EPOLLIN\n";
			}
			if (iter->events & EPOLLOUT) {
				LOG_DEBUG << "epoll_wait: EPOLLOUT\n";
			}
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

		break;
	}
	freeaddrinfo(list);
	if (newFd == -1) {
		LOG_ERROR << "initServer(): Cannot bind to a valid socket.\n";
		return false;
	}

	char bufHost[NI_MAXHOST];
	char bufPort[NI_MAXSERV];
	getnameinfo(list->ai_addr, list->ai_addrlen, bufHost, NI_MAXHOST, bufPort, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);
	const Connection connection = { newFd, bufHost, bufPort };
	IEndpoint* endpoint = new ServerEndpoint(connection);
	epoll_event event = {};
	event.events = EPOLLIN | EPOLLET;
	event.data.ptr = static_cast<void *>(endpoint);
	if (!addEvent(newFd, &event, endpoint)) {
		close(newFd);
		delete endpoint;
		return false;
	}
	LOG_INFO << "Added server endpoint: " << connection.host << ':' << connection.port;
	return true;
}
