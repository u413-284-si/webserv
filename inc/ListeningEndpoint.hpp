#pragma once

#include <cerrno>
#include <cstring>
#include <netdb.h>
#include <string>
#include <sys/epoll.h>
#include <unistd.h>

#include "IEndpoint.hpp"
#include "Log.hpp"
#include "Socket.hpp"

class Dispatcher;

class ListeningEndpoint : public IEndpoint {
public:
	explicit ListeningEndpoint(const Socket& serverSock);
	virtual ~ListeningEndpoint();

	virtual void handleEvent(Dispatcher& dispatcher, uint32_t eventMask);

	virtual time_t getTimeSinceLastEvent() const;

private:
	Socket m_serverSock;

	// Not copyable
	ListeningEndpoint(const ListeningEndpoint& ref);
	ListeningEndpoint& operator=(const ListeningEndpoint& ref);
};
