#pragma once

#include "Socket.hpp"
#include <ctime>
#include <unistd.h>

class Connection {
public:
	Connection(const Socket& server, const Socket& client);

	time_t getTimeSinceLastEvent() const;
	bool isActive() const;
	Socket getClient() const;
	Socket getServer() const;

private:
	Socket m_server;
	Socket m_client;
	time_t m_TimeSinceLastEvent;
	bool m_isActive;
};
