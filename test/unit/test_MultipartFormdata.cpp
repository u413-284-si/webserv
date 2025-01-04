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

// VALID HEADER TEST SUITE

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
