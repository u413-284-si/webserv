#pragma once

#include "IEndpoint.hpp"
#include "Connection.hpp"
#include <string>

class ServerEndpoint : public IEndpoint {
	public:
		ServerEndpoint(const Connection& connection);
		virtual ~ServerEndpoint();

		virtual void handleEvent(Dispatcher& dispatcher);

	private:
		Connection m_connection;
};
