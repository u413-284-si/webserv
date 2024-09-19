#include <gtest/gtest.h>
#include <string>

#include "CGIHandler.hpp"
#include "HTTPRequest.hpp"
#include "Socket.hpp"

class CGIHandlerTest : public ::testing::Test {
    protected:

    CGIHandlerTest() : m_cgiHandler(m_cgiPath, m_cgiExt) { }
    ~CGIHandlerTest() override { }

    std::string m_cgiPath = "/usr/bin/python3";
    std::string m_cgiExt = ".py";
    const Socket clientSock = { "1.23.4.56", "36952" };
    const Socket serverSock = { "127.0.0.1", "8080" };
    HTTPRequest request;
    
    CGIHandler m_cgiHandler;

    
};

TEST_F(CGIHandlerTest, Init)
{
    // Arrange
    Socket clientSocket;
}