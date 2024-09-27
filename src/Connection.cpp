#include "Connection.hpp"

/**
 * @brief Construct a new Connection:: Connection object
 *
 * The time since last event is initialized to the current time.
 * The status is initialized to Idle.
 * Tries to find a valid server configuration for the connection with hasValidServerConfig().
 * If no valid server configuration is found, the connection state is Connection::Closed.
 *
 * @param server Server socket associated with connection
 * @param client Client socket associated with connection
 */
Connection::Connection(const Socket& server, const Socket& client, int clientFd, const std::vector<ConfigServer>& serverConfigs)
	: m_serverSocket(server)
	, m_clientSocket(client)
	, m_clientFd(clientFd)
	, m_timeSinceLastEvent(std::time(0))
	, m_status(Idle)
	, m_pipeToCGIWriteEnd(-1)
	, m_pipeFromCGIReadEnd(-1)
	, m_cgiPid(-1)

{
	if (!hasValidServerConfig(*this, serverConfigs))
		m_status = Closed;
}

/**
 * @brief Clears the state of a given Connection object and resets its attributes.
 *
 * This function resets the status, request, buffer, and other attributes of the 
 * provided Connection object. It also closes any open file descriptors associated 
 * with CGI communication and resets the CGI process ID. Finally, it updates the 
 * time since the last event and checks if the connection has a valid server configuration.
 *
 * @param connection The Connection object to be cleared and reset.
 * @param serverConfigs A vector of ConfigServer objects to validate the connection against.
 * @return true if the connection has a valid server configuration, false otherwise.
 */
bool clearConnection(Connection& connection, const std::vector<ConfigServer>& serverConfigs)
{
	connection.m_status = Connection::Idle;
	connection.m_request = HTTPRequest();
	connection.m_buffer.clear();
	if (connection.m_pipeToCGIWriteEnd != -1) {
		close(connection.m_pipeToCGIWriteEnd);
		connection.m_pipeToCGIWriteEnd = -1;
	}
	if (connection.m_pipeFromCGIReadEnd != -1) {
		close(connection.m_pipeFromCGIReadEnd);
		connection.m_pipeFromCGIReadEnd = -1;
	}
	connection.m_cgiPid = -1;
	connection.m_timeSinceLastEvent = std::time(0);
	return (hasValidServerConfig(connection, serverConfigs));
}

/**
 * @brief Find matching server configurations for a server socket.
 *
 * Iterates through all server configurations and checks if the host and port match the server socket.
 * If no matching server is found, it tries to match the host with a wildcard server ("0.0.0.0"), the port must match.
 *
 * @param serverConfigs The vector of server configurations.
 * @param serverSock The server socket to find a matching server configuration for.
 * @return std::vector<std::vector<ConfigServer>::const_iterator> A vector of matching server configurations.
 */
std::vector<std::vector<ConfigServer>::const_iterator> findMatchingServerConfigs(
	const std::vector<ConfigServer>& serverConfigs, const Socket& serverSock)
{
	std::vector<std::vector<ConfigServer>::const_iterator> matches;

	for (std::vector<ConfigServer>::const_iterator iter = serverConfigs.begin(); iter != serverConfigs.end(); ++iter) {
		if (iter->host == serverSock.host && iter->port == serverSock.port) {
			matches.push_back(iter);
		}
	}

	if (!matches.empty())
		return matches;

	const std::string wildcard = "0.0.0.0";

	for (std::vector<ConfigServer>::const_iterator iter = serverConfigs.begin(); iter != serverConfigs.end(); ++iter) {
		if (iter->host == wildcard && iter->port == serverSock.port) {
			matches.push_back(iter);
		}
	}
	return matches;
}

/**
 * @brief Selects the server configuration for a connection.
 *
 * Finds possible server configurations by calling findMatchingServerConfigs().
 * If no matching server configuration is found, returns false.
 * If more than one matching server is found, sets the first one found.
 * Also sets the first location of the found server configuration.
 *
 * @param connection The connection object to select the server configuration for.
 * @param serverConfigs The vector of server configurations.
 * @return true if a valid server configuration is found, false otherwise.
 */
bool hasValidServerConfig(Connection& connection, const std::vector<ConfigServer>& serverConfigs)
{
	std::vector<std::vector<ConfigServer>::const_iterator> matches
		= findMatchingServerConfigs(serverConfigs, connection.m_serverSocket);

	if (matches.empty())
		return (false);

	connection.serverConfig = matches[0];
	connection.location = matches[0]->locations.begin();

	return (true);
}

/**
 * @brief Overload for hasValidServerConfig() to aditionally check a server configuration for host.
 *
 * Works similar to hasValidServerConfig(). If after finding all matching server configurations more than one match was
 * found, it iterates through the matches and checks if the server name matches the host.
 * If it does, sets the found server config. Also sets the first location of the found server configuration.
 * If no server name matches the host, the first found match is kept as matching server.
 *
 * @param connection The connection object to select the server configuration for.
 * @param serverConfigs The vector of server configurations.
 * @param host The host name to match with the server name.
 * @return true if a valid server configuration is found, false otherwise.
 */
bool hasValidServerConfig(
	Connection& connection, const std::vector<ConfigServer>& serverConfigs, const std::string& host)
{
	std::vector<std::vector<ConfigServer>::const_iterator> matches
		= findMatchingServerConfigs(serverConfigs, connection.m_serverSocket);

	if (matches.empty())
		return (false);

	connection.serverConfig = matches[0];
	connection.location = matches[0]->locations.begin();
	if (matches.size() == 1)
		return (true);

	for (std::vector<std::vector<ConfigServer>::const_iterator>::const_iterator iter = matches.begin();
		 iter != matches.end(); ++iter) {
		if ((*iter)->serverName == host) {
			connection.serverConfig = *iter;
			connection.location = (*iter)->locations.begin();
			return (true);
		}
	}

	return (true);
}
