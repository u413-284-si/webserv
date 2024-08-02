#pragma once

#include "Socket.hpp"
#include <ctime>
#include <unistd.h>

class Connection {
public:
	Connection(const Socket& server, const Socket& client);

	enum ConnectionStatus { ReceiveRequest, ReceiveBody, BuildResponse, SendResponse };

	void setStatus(ConnectionStatus status);
	void closeConnection() const;

	Socket getServer() const;
	Socket getClient() const;
	time_t getTimeSinceLastEvent() const;
	ConnectionStatus getStatus() const;

private:
	Socket m_server;
	Socket m_client;
	time_t m_TimeSinceLastEvent;
	ConnectionStatus m_status;
};
