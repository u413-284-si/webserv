#include "RequestParser.hpp"
#include "gtest/gtest.h"

class ParseBodyTest : public ::testing::Test {
protected:
	ParseBodyTest()
		: request()
	{
		request.hasBody = true;
		request.isChunked = false;
	}
	~ParseBodyTest() override { }

    std::vector<char> buffer;
	RequestParser p;
	HTTPRequest request;
};

// VALID BODY TEST SUITE

TEST_F(ParseBodyTest, ChunkedBody)
{
	// Arrange
	request.isChunked = true;

	// Act
	p.parseBody("6\r\nhello \r\n6\r\nworld!\r\n0\r\n\r\n", request, buffer);

	// Assert
	EXPECT_EQ(request.body, "hello world!");
}

TEST_F(ParseBodyTest, NonChunkedBodySize14)
{
	// Arrange
	request.headers["content-length"] = "14";

	// Act
	p.parseBody("hello \r\nworld!", request, buffer);

	// Assert
	EXPECT_EQ(request.body, "hello \nworld!");
}

TEST_F(ParseBodyTest, NonChunkedBodySize16)
{
	// Arrange
	request.headers["content-length"] = "16";

	// Act
	p.parseBody("hello \r\nworld!\r\n", request, buffer);

	// Assert
	EXPECT_EQ(request.body, "hello \nworld!\n");
}

// INVALID BODY TEST SUITE

TEST_F(ParseBodyTest, DifferingChunkSize)
{
	// Arrange
	request.isChunked = true;

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.parseBody("1\r\nhello\r\n6\r\nworld!\r\n0\r\n\r\n", request, buffer);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ("Invalid HTTP request: missing CRLF", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(ParseBodyTest, DifferingContentLength)
{
	// Arrange
	request.headers["content-length"] = "3";

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.parseBody("hello \r\nworld!\r\n", request, buffer);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ(
					"Invalid HTTP request: Indicated content length different than actual body size", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(ParseBodyTest, MissingCRLFInChunk)
{
	// Arrange
	request.isChunked = true;

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.parseBody("6\r\nhello 6\r\nworld!\r\n0\r\n\r\n", request, buffer);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ("Invalid HTTP request: missing CRLF", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(ParseBodyTest, MissingCRInChunk)
{
	// Arrange
	request.isChunked = true;

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.parseBody("6\r\nhello \n6\r\nworld!\r\n0\r\n\r\n", request, buffer);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ("Invalid HTTP request: missing CRLF", e.what());
				throw;
			}
		},
		std::runtime_error);
}
