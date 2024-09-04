#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdexcept>

#include "Server.hpp"

using ::testing::Return;

class selectServerConfigTest : public ::testing::Test {
	protected:
	selectServerConfigTest()
	{
	}
	~selectServerConfigTest() override { }

	ConfigFile configFile;
	Socket serverSock = {
		.host = "127.0.0.1",
		.port = "8080"
	};
};

TEST_F(selectServerConfigTest, OneServer)
{
	ConfigServer serverConfig;
	serverConfig.host = "127.0.0.1";
	serverConfig.port = "8080";

	configFile.servers.push_back(serverConfig);

	EXPECT_EQ(selectServerConfig(configFile.servers, serverSock), configFile.servers.begin());
}

TEST_F(selectServerConfigTest, OneServerWildcard)
{
	ConfigServer serverConfig;
	serverConfig.host = "0.0.0.0";
	serverConfig.port = "8080";

	configFile.servers.push_back(serverConfig);

	EXPECT_EQ(selectServerConfig(configFile.servers, serverSock), configFile.servers.begin());
}

TEST_F(selectServerConfigTest, MultipleServers)
{
	ConfigServer serverConfig1;
	serverConfig1.host = "192.168.0.1";
	serverConfig1.port = "8080";

	ConfigServer serverConfig2;
	serverConfig2.host = "127.0.0.1";
	serverConfig2.port = "8080";

	ConfigServer serverConfig3;
	serverConfig3.host = "10.0.0.1";
	serverConfig3.port = "8080";

	configFile.servers.push_back(serverConfig1);
	configFile.servers.push_back(serverConfig2);
	configFile.servers.push_back(serverConfig3);

	EXPECT_EQ(selectServerConfig(configFile.servers, serverSock), configFile.servers.begin() + 1);
}

TEST_F(selectServerConfigTest, MultipleServersOneWildcard)
{
	ConfigServer serverConfig1;
	serverConfig1.host = "192.168.0.1";
	serverConfig1.port = "8080";

	ConfigServer serverConfig2;
	serverConfig2.host = "0.0.0.0";
	serverConfig2.port = "8080";

	ConfigServer serverConfig3;
	serverConfig3.host = "127.0.0.1";
	serverConfig3.port = "8080";

	configFile.servers.push_back(serverConfig1);
	configFile.servers.push_back(serverConfig2);
	configFile.servers.push_back(serverConfig3);

	EXPECT_EQ(selectServerConfig(configFile.servers, serverSock), configFile.servers.begin() + 2);
}

TEST_F(selectServerConfigTest, MultipleMatchesNoHost)
{
	ConfigServer serverConfig1;
	serverConfig1.host = "127.0.0.1";
	serverConfig1.port = "8080";

	ConfigServer serverConfig2;
	serverConfig2.host = "127.0.0.1";
	serverConfig2.port = "8080";

	ConfigServer serverConfig3;
	serverConfig3.host = "127.0.0.1";
	serverConfig3.port = "8080";

	configFile.servers.push_back(serverConfig1);
	configFile.servers.push_back(serverConfig2);
	configFile.servers.push_back(serverConfig3);

	EXPECT_EQ(selectServerConfig(configFile.servers, serverSock), configFile.servers.begin());
}

TEST_F(selectServerConfigTest, MultipleMatchesWithHost)
{
	ConfigServer serverConfig1;
	serverConfig1.host = "127.0.0.1";
	serverConfig1.port = "8080";
	serverConfig1.serverName = "server1";

	ConfigServer serverConfig2;
	serverConfig2.host = "127.0.0.1";
	serverConfig2.port = "8080";
	serverConfig2.serverName = "server2";

	ConfigServer serverConfig3;
	serverConfig3.host = "127.0.0.1";
	serverConfig3.port = "8080";
	serverConfig3.serverName = "server3";

	configFile.servers.push_back(serverConfig1);
	configFile.servers.push_back(serverConfig2);
	configFile.servers.push_back(serverConfig3);

	EXPECT_EQ(selectServerConfig(configFile.servers, serverSock, "server3"), configFile.servers.begin() + 2);
}

TEST_F(selectServerConfigTest, SingleMatchWrongHost)
{
	ConfigServer serverConfig1;
	serverConfig1.host = "192.168.0.1";
	serverConfig1.port = "8080";
	serverConfig1.serverName = "server1";

	ConfigServer serverConfig2;
	serverConfig2.host = "127.0.0.1";
	serverConfig2.port = "8080";
	serverConfig2.serverName = "server2";

	ConfigServer serverConfig3;
	serverConfig3.host = "10.0.0.1";
	serverConfig3.port = "8080";
	serverConfig3.serverName = "server3";

	configFile.servers.push_back(serverConfig1);
	configFile.servers.push_back(serverConfig2);
	configFile.servers.push_back(serverConfig3);

	EXPECT_EQ(selectServerConfig(configFile.servers, serverSock, "wrong"), configFile.servers.begin() + 1);
}

TEST_F(selectServerConfigTest, MultipleMatchesWithNotMatchingHost)
{
	ConfigServer serverConfig1;
	serverConfig1.host = "127.0.0.1";
	serverConfig1.port = "8080";
	serverConfig1.serverName = "server1";

	ConfigServer serverConfig2;
	serverConfig2.host = "127.0.0.1";
	serverConfig2.port = "8080";
	serverConfig2.serverName = "server2";

	ConfigServer serverConfig3;
	serverConfig3.host = "127.0.0.1";
	serverConfig3.port = "8080";
	serverConfig3.serverName = "server3";

	configFile.servers.push_back(serverConfig1);
	configFile.servers.push_back(serverConfig2);
	configFile.servers.push_back(serverConfig3);

	EXPECT_EQ(selectServerConfig(configFile.servers, serverSock, "wrong"), configFile.servers.begin());
}

TEST_F(selectServerConfigTest, NoMatch)
{
	ConfigServer serverConfig1;
	serverConfig1.host = "192.168.0.1";
	serverConfig1.port = "1234";
	serverConfig1.serverName = "server1";

	configFile.servers.push_back(serverConfig1);

	EXPECT_THROW(selectServerConfig(configFile.servers, serverSock), std::runtime_error);
}

TEST_F(selectServerConfigTest, NoMatchWithHost)
{
	ConfigServer serverConfig1;
	serverConfig1.host = "192.168.0.1";
	serverConfig1.port = "1234";
	serverConfig1.serverName = "server1";

	configFile.servers.push_back(serverConfig1);

	EXPECT_THROW(selectServerConfig(configFile.servers, serverSock, "wrong"), std::runtime_error);
}
