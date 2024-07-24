#pragma once

#include <cerrno>
#include <netdb.h>
#include <string.h>
#include <string>
#include <sys/epoll.h>
#include <unistd.h>

#include "Connection.hpp"
#include "IEndpoint.hpp"
#include "Log.hpp"

class Dispatcher;

class ServerEndpoint : public IEndpoint {
public:
	ServerEndpoint(const Connection& connection);
	virtual ~ServerEndpoint();

	virtual void handleEvent(Dispatcher& dispatcher);

	virtual time_t getTimeSinceLastEvent() const;
	virtual void setTimeSinceLastEvent();

private:
	Connection m_connection;
};
