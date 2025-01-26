#include "test_helpers.hpp"

using ::testing::Return;

class isDuplicateServerTest : public ServerTestBase {
	protected:
	isDuplicateServerTest()
	{
		ON_CALL(m_epollWrapper, addEvent)
		.WillByDefault(Return(true));
	}
	~isDuplicateServerTest() override { }

};

TEST_F(isDuplicateServerTest, EmptyMap)
{
	std::string host = "127.0.0.1";
	std::string port = "8080";

	EXPECT_EQ(isDuplicateServer(m_server, host, port), false);
}

TEST_F(isDuplicateServerTest, NoDuplicate)
{
	m_server.registerVirtualServer(10, (Socket { "127.0.0.1", "7070" }));
	m_server.registerVirtualServer(20, (Socket { "192.168.0.1", "8080" }));

	std::string host = "127.0.0.1";
	std::string port = "8080";

	EXPECT_EQ(isDuplicateServer(m_server, host, port), false);
}

TEST_F(isDuplicateServerTest, DuplicateServer)
{
	m_server.registerVirtualServer(10, (Socket { "127.0.0.1", "6060" }));
	m_server.registerVirtualServer(20, (Socket { "127.0.0.1", "7070" }));
	m_server.registerVirtualServer(30, (Socket { "127.0.0.1", "8080" }));

	std::string host = "127.0.0.1";
	std::string port = "8080";

	EXPECT_EQ(isDuplicateServer(m_server, host, port), true);
}

TEST_F(isDuplicateServerTest, DuplicateServerLocalhost)
{
	m_server.registerVirtualServer(10, (Socket { "127.0.0.1", "6060" }));
	m_server.registerVirtualServer(20, (Socket { "127.0.0.1", "7070" }));
	m_server.registerVirtualServer(30, (Socket { "127.0.0.1", "8080" }));
	m_server.registerVirtualServer(40, (Socket { "::1", "9090" }));
	m_server.registerVirtualServer(50, (Socket { "::1", "1010" }));

	std::string host = "localhost";
	std::string port = "7070";
	std::string port2 = "2020";
	std::string port3 = "1010";

	EXPECT_EQ(isDuplicateServer(m_server, host, port), true);
	EXPECT_EQ(isDuplicateServer(m_server, host, port2), false);
	EXPECT_EQ(isDuplicateServer(m_server, host, port3), true);
}
