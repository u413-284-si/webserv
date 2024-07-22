#pragma once

#include "IEndpoint.hpp"
#include "Connection.hpp"
#include "Dispatcher.hpp"
#include "Log.hpp"
#include <string>
#include <unistd.h>
#include <netdb.h>
#include <cerrno>
#include <string.h>
#include <sys/epoll.h>

class ServerEndpoint : public IEndpoint {
	public:
		ServerEndpoint(const Connection& connection);
		virtual ~ServerEndpoint();

		virtual void handleEvent(Dispatcher& dispatcher);

	private:
		Connection m_connection;
};
