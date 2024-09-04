#include <gmock/gmock.h>
#include <gtest/gtest.h>

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