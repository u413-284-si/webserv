#pragma once

#include <string>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <cerrno>
#include <string.h>
#include <sstream>

#include "Connection.hpp"
#include "IEndpoint.hpp"
#include "Log.hpp"

class ClientEndpoint : public IEndpoint {
public:
	ClientEndpoint(const Connection& connection, const Connection& server);
	virtual ~ClientEndpoint();

	virtual void handleEvent(Dispatcher& dispatcher, uint32_t eventMask);

	virtual time_t getTimeSinceLastEvent() const;
	virtual void setTimeSinceLastEvent();

private:
	Connection m_connection;
	Connection m_server;
	time_t m_TimeSinceLastEvent;
	std::string m_buffer;
};
