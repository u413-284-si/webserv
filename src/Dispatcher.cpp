#include "Dispatcher.hpp"

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

Dispatcher::~Dispatcher()
{
	for (std::vector<struct epoll_event>::iterator iter = m_events.begin(); iter != m_events.end(); ++iter) {
		if (iter->data.fd != -1) {
			close(iter->data.fd);
		}
	}
	close(m_epfd);
}

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

void Dispatcher::addEvent(const int newfd, epoll_event* event) const
{
	if (epoll_ctl(m_epfd, EPOLL_CTL_ADD, newfd, event) == -1) {
		LOG_ERROR << "epoll_ctl: EPOLL_CTL_ADD: " << strerror(errno) << '\n';
	}
	LOG_DEBUG << "epoll_ctl: Added new fd: " << newfd << '\n';
}

void Dispatcher::removeEvent(const int delfd) const
{
	if (epoll_ctl(m_epfd, EPOLL_CTL_DEL, delfd, NULL) == -1) {
		LOG_ERROR << "epoll_ctl: EPOLL_CTL_DEL: " << strerror(errno) << '\n';
	}
	LOG_DEBUG << "epoll_ctl: Removed fd: " << delfd << '\n';
}

void Dispatcher::modifyEvent(const int modfd, epoll_event* event) const
{
	if (epoll_ctl(m_epfd, EPOLL_CTL_MOD, modfd, event) == -1) {
		LOG_ERROR << "epoll_ctl: EPOLL_CTL_MOD: " << strerror(errno) << '\n';
	}
	LOG_DEBUG << "epoll_ctl: Modified fd: " << modfd << '\n';
}

int Dispatcher::wait()
{
	const int nfds = epoll_wait(m_epfd, &m_events[0], static_cast<int>(m_events.size()), m_timeout);
	if (nfds == -1)
		LOG_ERROR << "epoll_wait: " << strerror(errno) << '\n';
	else if (nfds == 0)
		LOG_DEBUG << "epoll_wait: Timeout\n";
	else
		LOG_DEBUG << "epoll_wait: " << nfds << " events\n";
	return nfds;
}
