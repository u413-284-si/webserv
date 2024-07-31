#pragma once

#include <cerrno>
#include <cstring>
#include <sstream>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include "IEndpoint.hpp"
#include "Log.hpp"
#include "Socket.hpp"

class ConnectedEndpoint : public IEndpoint {
public:
	explicit ConnectedEndpoint(const Connection& connection);
	virtual ~ConnectedEndpoint();

	virtual void handleEvent(Dispatcher& dispatcher, uint32_t eventMask);

	virtual time_t getTimeSinceLastEvent() const;
	virtual bool isActive() const;

private:
	Socket m_clientSock;
	Socket m_serverSock;
	time_t m_TimeSinceLastEvent;
	std::string m_buffer;
	bool m_isActive;

	void setTimeSinceLastEvent();
	void closeConnection(Dispatcher& dispatcher);

	// Not copyable
	ConnectedEndpoint(const ConnectedEndpoint& ref);
	ConnectedEndpoint& operator=(const ConnectedEndpoint& ref);
};
