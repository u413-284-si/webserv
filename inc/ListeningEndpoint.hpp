#pragma once

#include <cerrno>
#include <netdb.h>
#include <string.h>
#include <string>
#include <sys/epoll.h>
#include <unistd.h>

#include "Socket.hpp"
#include "IEndpoint.hpp"
#include "Log.hpp"

class Dispatcher;

class ListeningEndpoint : public IEndpoint {
public:
	ListeningEndpoint(const Socket& serverSock);
	virtual ~ListeningEndpoint();

	virtual void handleEvent(Dispatcher& dispatcher, uint32_t eventMask);

	virtual time_t getTimeSinceLastEvent() const;

private:
	Socket m_serverSock;
};
