#pragma once

#include "Socket.hpp"
#include <cstddef>
#include <ctime>
#include <unistd.h>

class Connection {
public:
	Connection(const Socket& server, const Socket& client);

	enum ConnectionStatus { ReceiveRequest, ReceiveBody, BuildResponse, SendResponse };

	void setStatus(ConnectionStatus status);
	void closeConnection() const;
	void updateBytesReceived(std::size_t bytesReceived);

	Socket getServer() const;
	Socket getClient() const;
	time_t getTimeSinceLastEvent() const;
	ConnectionStatus getStatus() const;
	std::size_t getBytesReceived() const;

private:
	Socket m_server;
	Socket m_client;
	time_t m_TimeSinceLastEvent;
	ConnectionStatus m_status;
	std::size_t m_bytesReceived;
};
