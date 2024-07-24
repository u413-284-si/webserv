#pragma once

#include "Connection.hpp"
#include "IEndpoint.hpp"
#include <string>
#include <unistd.h>

class ClientEndpoint : public IEndpoint {
public:
	ClientEndpoint(const Connection& connection, const Connection& server);
	virtual ~ClientEndpoint();

	virtual void handleEvent(Dispatcher& dispatcher);

	virtual time_t getTimeSinceLastEvent() const;
	virtual void setTimeSinceLastEvent();

private:
	Connection m_connection;
	Connection m_server;
	time_t m_TimeSinceLastEvent;
};
