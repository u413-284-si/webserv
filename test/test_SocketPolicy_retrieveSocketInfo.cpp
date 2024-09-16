#include <gtest/gtest.h>

#include "SocketPolicy.hpp"
#include "arpa/inet.h"

TEST(SocketPolicy_retrieveSocketInfo, IPv4)
{
	struct sockaddr_in addr = {
		.sin_family = AF_INET,
		.sin_port = htons(80)
	};
	inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

	char temp[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &addr.sin_addr, temp, INET_ADDRSTRLEN);

	SocketPolicy policy;
	Socket test = policy.retrieveSocketInfo(reinterpret_cast<struct sockaddr*>(&addr));

	EXPECT_EQ(test.host, temp);
	EXPECT_EQ(test.port, "80");
}

TEST(SocketPolicy_retrieveSocketInfo, IPv4AllZero)
{
	struct sockaddr_in addr = {
		.sin_family = AF_INET,
		.sin_port = htons(10101)
	};
	inet_pton(AF_INET, "0.0.0.0", &addr.sin_addr);

	char temp[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &addr.sin_addr, temp, INET_ADDRSTRLEN);

	SocketPolicy policy;
	Socket test = policy.retrieveSocketInfo(reinterpret_cast<struct sockaddr*>(&addr));

	EXPECT_EQ(test.host, temp);
	EXPECT_EQ(test.port, "10101");
}

TEST(SocketPolicy_retrieveSocketInfo, IPv4NonZero)
{
	struct sockaddr_in addr = {
		.sin_family = AF_INET,
		.sin_port = htons(65535)
	};
	inet_pton(AF_INET, "123.255.9.66", &addr.sin_addr);

	char temp[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &addr.sin_addr, temp, INET_ADDRSTRLEN);

	SocketPolicy policy;
	Socket test = policy.retrieveSocketInfo(reinterpret_cast<struct sockaddr*>(&addr));

	EXPECT_EQ(test.host, temp);
	EXPECT_EQ(test.port, "65535");
}
