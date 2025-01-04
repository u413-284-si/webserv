#include "RequestParser.hpp"
#include "gtest/gtest.h"
#include <stdexcept>

class MultipartFormdataTest : public ::testing::Test {
protected:
	MultipartFormdataTest()
		: request()
	{
		request.hasMultipartFormdata = false;
	}
	~MultipartFormdataTest() override { }

	RequestParser p;
	HTTPRequest request;
};

TEST_F(MultipartFormdataTest, ParseHeader)
{
	// Arrange
	const std::string headerString = "POST /upload HTTP/1.1\r\nHost: example.com\r\nContent-Type: multipart/form-data; "
									 "boundary=----WebKitFormBoundary7MA4YWxkTrZu0gW\r\nContent-Length: 195";
	// Act
	p.parseHeader(headerString, request);

	// Assert
	EXPECT_TRUE(request.hasMultipartFormdata);
	EXPECT_EQ(p.getBoundary(), "WebKitFormBoundary7MA4YWxkTrZu0gW");
}

TEST_F(MultipartFormdataTest, ParseHeaderNoBoundary)
{
	// Arrange
	const std::string headerString = "POST /upload HTTP/1.1\r\nHost: example.com\r\nContent-Type: multipart/form-data\r\n"
									 "Content-Length: 195";
	// Act & Assert
	EXPECT_THROW(
	{
		try {
			p.parseHeader(headerString, request);
		} catch (const std::runtime_error& e) {
			EXPECT_STREQ(ERR_BAD_MULTIPART_FORMDATA, e.what());
			EXPECT_EQ(request.shallCloseConnection, true);
			throw;
		}
	},
	std::runtime_error);
}
