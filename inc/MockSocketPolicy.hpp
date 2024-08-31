#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "SocketPolicy.hpp"

/**
 * @brief Mock class for SocketPolicy.
 *
 */
class MockSocketPolicy : public SocketPolicy {
public:
	MOCK_METHOD(
		struct addrinfo*, resolveListeningAddresses, (const std::string&, const std::string&), (const, override));
	MOCK_METHOD(int, createListeningSocket, (const struct addrinfo&, int), (const, override));
	MOCK_METHOD(Socket, retrieveSocketInfo, (struct sockaddr&, socklen_t), (const, override));
	MOCK_METHOD(int, acceptSingleConnection, (int, struct sockaddr*, socklen_t*), (const, override));
	MOCK_METHOD(ssize_t, readFromSocket, (int, char*, size_t, int), (const, override));
	MOCK_METHOD(ssize_t, writeToSocket, (int, const char*, size_t, int), (const, override));
};
