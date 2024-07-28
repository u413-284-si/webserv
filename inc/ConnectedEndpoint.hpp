#pragma once

#include <string>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <cerrno>
#include <string.h>
#include <sstream>

#include "Socket.hpp"
#include "IEndpoint.hpp"
#include "Log.hpp"

class ConnectedEndpoint : public IEndpoint {
public:
	ConnectedEndpoint(const Socket& clientSock, const Socket& serverSock);
	virtual ~ConnectedEndpoint();

	virtual void handleEvent(Dispatcher& dispatcher, uint32_t eventMask);

	virtual time_t getTimeSinceLastEvent() const;
	virtual void setTimeSinceLastEvent();

private:
	Socket m_clientSock;
	Socket m_serverSock;
	time_t m_TimeSinceLastEvent;
	std::string m_buffer;
};
