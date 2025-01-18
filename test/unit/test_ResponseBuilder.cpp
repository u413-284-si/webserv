#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "MockFileSystemPolicy.hpp"
#include "ResponseBuilder.hpp"

ConfigFile createTestConfigfile();

using ::testing::Return;
using ::testing::HasSubstr;

class ResponseBuilderTest : public ::testing::Test {
	protected:
	ResponseBuilderTest()
	{

	}
	~ResponseBuilderTest() override { }

	const int m_dummyFd = 10;
	Socket m_serverSock = { .host = "127.0.0.1", .port = "8080" };
	ConfigFile m_configFile = createTestConfigfile();
	Connection m_connection = Connection(m_serverSock, Socket(), m_dummyFd, m_configFile.servers);

	HTTPRequest& m_request = m_connection.m_request;

	std::string m_responseBody;
	std::map<std::string, std::string> m_responseHeaders;
	MockFileSystemPolicy m_fileSystemPolicy;
	ResponseBuilder m_responseBuilder = ResponseBuilder(m_fileSystemPolicy);
};

TEST_F(ResponseBuilderTest, SimpleResponse)
{
	EXPECT_CALL(m_fileSystemPolicy, getFileContents)
		.WillOnce(Return("content"));

	m_request.method = MethodGet;
	m_request.targetResource = "/index.html";

	m_responseBuilder.buildResponse(m_connection);

	std::string response = m_responseBuilder.getResponse();

	EXPECT_THAT(response, HasSubstr("HTTP/1.1 200 OK\r\n"));
	EXPECT_THAT(response, HasSubstr("Content-Type: text/html\r\n"));
	EXPECT_THAT(response, HasSubstr("Content-Length: 7\r\n"));
	EXPECT_THAT(response, HasSubstr("Server: TriHard\r\n"));
	EXPECT_THAT(response, HasSubstr("Date: "));
	EXPECT_THAT(response, HasSubstr("Connection: keep-alive\r\n"));
	EXPECT_THAT(response, HasSubstr("\r\n\r\ncontent"));
}

TEST_F(ResponseBuilderTest, Redirection)
{
	m_request.method = MethodGet;
	m_request.httpStatus = StatusPermanentRedirect;
	m_request.targetResource = "/redirect.html";

	m_responseBuilder.buildResponse(m_connection);

	std::string response = m_responseBuilder.getResponse();

	EXPECT_THAT(response, HasSubstr("HTTP/1.1 308 Permanent Redirect\r\n"));
	EXPECT_THAT(response, HasSubstr("Content-Type: text/html\r\n"));
	EXPECT_THAT(response, HasSubstr("Location: /redirect.html\r\n"));
	EXPECT_THAT(response, HasSubstr("Connection: keep-alive\r\n"));
}

TEST_F(ResponseBuilderTest, MethodNotAllowed)
{
	m_request.method = MethodGet;
	m_request.targetResource = "/not_allowed.html";
	m_request.httpStatus = StatusMethodNotAllowed;
	m_request.shallCloseConnection = true;

	m_configFile.servers[0].locations[0].allowedMethods[MethodGet] = false;
	m_configFile.servers[0].locations[0].allowedMethods[MethodPost] = true;
	m_configFile.servers[0].locations[0].allowedMethods[MethodDelete] = true;

	m_responseBuilder.buildResponse(m_connection);

	std::string response = m_responseBuilder.getResponse();

	EXPECT_THAT(response, HasSubstr("HTTP/1.1 405 Method Not Allowed\r\n"));
	EXPECT_THAT(response, HasSubstr("Content-Type: text/html\r\n"));
	EXPECT_THAT(response, HasSubstr("Allow: POST, DELETE\r\n"));
	EXPECT_THAT(response, HasSubstr("Connection: close\r\n"));
}
