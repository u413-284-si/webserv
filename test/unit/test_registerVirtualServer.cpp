#include "test_helpers.hpp"

using ::testing::Return;

class RegisterVirtualServerTest : public ServerTestBase {
	protected:
	RegisterVirtualServerTest() { }
	~RegisterVirtualServerTest() override { }

	Socket socket = {
		"127.0.0.1",
		"8080" };
	Socket socket2 = {
		"192.168.0.1",
		"7070" };
	const int dummyFd = 10;
};

TEST_F(RegisterVirtualServerTest, ServerRegisterSuccess)
{
	EXPECT_CALL(m_epollWrapper, addEvent)
	.Times(1)
	.WillOnce(Return(true));

	EXPECT_EQ(m_server.registerVirtualServer(dummyFd, socket), true);
	EXPECT_EQ(m_server.getVirtualServers().size(), 1);
	EXPECT_EQ(m_server.getVirtualServers().at(dummyFd).host, socket.host);
	EXPECT_EQ(m_server.getVirtualServers().at(dummyFd).port, socket.port);
}

TEST_F(RegisterVirtualServerTest, ServerRegisterFail)
{
	EXPECT_CALL(m_epollWrapper, addEvent)
	.Times(1)
	.WillOnce(Return(false));

	EXPECT_EQ(m_server.registerVirtualServer(dummyFd, socket), false);
	EXPECT_EQ(m_server.getVirtualServers().size(), 0);
}

TEST_F(RegisterVirtualServerTest, ServerAlreadyPresent)
{
	// epoll will fail the second time to add the event because the fd is already present
	EXPECT_CALL(m_epollWrapper, addEvent)
	.Times(2)
	.WillOnce(Return(true))
	.WillOnce(Return(false));

	EXPECT_EQ(m_server.registerVirtualServer(dummyFd, socket2), true);

	EXPECT_EQ(m_server.registerVirtualServer(dummyFd, socket), false);
	EXPECT_EQ(m_server.getVirtualServers().size(), 1);
	EXPECT_EQ(m_server.getVirtualServers().at(dummyFd).host, socket2.host);
	EXPECT_EQ(m_server.getVirtualServers().at(dummyFd).port, socket2.port);
}
