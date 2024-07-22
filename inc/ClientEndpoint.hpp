#pragma once

#include "IEndpoint.hpp"
#include "Connection.hpp"
#include <string>

class ClientEndpoint : public IEndpoint {
	public:
		ClientEndpoint(const Connection& connection, const Connection& server);
		virtual ~ClientEndpoint();

		virtual void handleEvent(Dispatcher& dispatcher);

	private:
		Connection m_connection;
		Connection m_server;
};
