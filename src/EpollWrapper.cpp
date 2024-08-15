#include "EpollWrapper.hpp"

/**
 * @brief Constructor for EpollWrapper.
 *
 * This constructor creates an epoll instance and initializes the epoll events vector.
 * @param maxEvents Maximum number of events for the epoll instance.
 * @param epollTimeout Timeout for the epoll instance in milliseconds.
 */
EpollWrapper::EpollWrapper(size_t maxEvents, int epollTimeout)
	: m_epfd(epoll_create(1))
	, m_events(maxEvents)
	, m_timeout(epollTimeout)
{
	if (m_epfd == -1) {
		throw std::runtime_error("Failed to create epoll instance: " + std::string(std::strerror(errno)));
	}
}

/**
 * @brief Destructor for EpollWrapper.
 *
 * This destructor closes the epoll instance.
 */
EpollWrapper::~EpollWrapper() { close(m_epfd); }

/**
 * @brief Wait for events on an epoll instance.
 *
 * If epoll_wait returns -1, it checks if the error is due to an interrupt signal.
 * If it is, it returns 0. Otherwise, it throws a runtime error with the error message.
 * If epoll_wait returns 0, it logs a timeout message. Otherwise, it logs the number of events.
 *
 * @return The number of events that occurred.

 * @throws std::runtime_error if epoll_wait() encounters an error.
 */
int EpollWrapper::waitForEvents()
{
	LOG_DEBUG << "Waiting for events";

	const int nfds = epoll_wait(m_epfd, &m_events[0], static_cast<int>(m_events.size()), m_timeout);
	if (nfds == -1) {
		if (errno == EINTR)
			return 0;
		throw std::runtime_error("epoll_wait:" + std::string(strerror(errno)));
	}

	if (nfds == 0)
		LOG_DEBUG << "epoll_wait: Timeout";
	else
		LOG_DEBUG << "epoll_wait: " << nfds << " events";
	return nfds;
}

/**
 * @brief Get the beginning of the events vector.
 *
 * @return A const iterator to the beginning of the events vector.
 */
std::vector<struct epoll_event>::const_iterator EpollWrapper::eventsBegin() const { return m_events.begin(); }

/**
 * @brief Add an event to the epoll instance.
 *
 * Calls epoll_ctl() with EPOLL_CTL_ADD to add a new event to the epoll instance.
 * It sets event.data.fd to newfd and event.events to eventMask.
 * If epoll_ctl returns -1, it logs an error message and returns false.
 *
 * @param newfd The file descriptor of the new event.
 * @param eventMask The eventmask of the new event to add.
 *
 * @return true if the event was successfully added, false otherwise.
 */
bool EpollWrapper::addEvent(int newfd, uint32_t eventMask) const
{
	struct epoll_event event = {};
	event.events = eventMask;
	event.data.fd = newfd;

	if (-1 == epoll_ctl(m_epfd, EPOLL_CTL_ADD, newfd, &event)) {
		LOG_ERROR << "epoll_ctl: EPOLL_CTL_ADD: " << strerror(errno);
		return false;
	}
	LOG_DEBUG << "epoll_ctl: Added new fd: " << newfd;

	return true;
}

/**
 * @brief Modify an event in the epoll instance.
 *
 * Calls epoll_ctl() with EPOLL_CTL_MOD to modify an existing event in the epoll instance.
 * It sets event.data.fd to modfd and event.events to eventMask.
 * If epoll_ctl() returns -1, it logs an error message and returns false.
 *
 * @param modfd The file descriptor of the event to modify.
 * @param eventMask The eventmask of the modified event.
 *
 * @return true if the event was successfully modified, false otherwise.
 */
bool EpollWrapper::modifyEvent(int modfd, uint32_t eventMask) const
{
	struct epoll_event event = {};
	event.events = eventMask;
	event.data.fd = modfd;

	if (-1 == epoll_ctl(m_epfd, EPOLL_CTL_MOD, modfd, &event)) {
		LOG_ERROR << "epoll_ctl: EPOLL_CTL_MOD: " << strerror(errno);
		return false;
	}
	LOG_DEBUG << "epoll_ctl: Modified fd: " << modfd;
	return true;
}

/**
 * @brief Remove an event from the epoll instance.
 *
 * Calls epoll_ctl() with EPOLL_CTL_DEL to remove an existing event from the epoll instance.
 * If epoll_ctl returns -1, it logs an error message.
 *
 * @param delfd The file descriptor of the event to remove.
 */
void EpollWrapper::removeEvent(int delfd) const
{
	if (-1 == epoll_ctl(m_epfd, EPOLL_CTL_DEL, delfd, NULL)) {
		LOG_ERROR << "epoll_ctl: EPOLL_CTL_DEL: " << strerror(errno);
		return;
	}
	LOG_DEBUG << "epoll_ctl: Removed fd: " << delfd;
}
