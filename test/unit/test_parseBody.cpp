#include "RequestParser.hpp"
#include "error.hpp"
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

TEST_F(ParseBodyTest, ChunkedBodyWithNewline)
{
	// Arrange
	request.isChunked = true;

	// Act
	p.parseBody("6\r\nhello \r\n8\r\nw\n\norld!\r\n0\r\n\r\n", request, buffer);

	// Assert
	EXPECT_EQ(request.body, "hello w\n\norld!");
}

TEST_F(ParseBodyTest, ChunkedBodyWithCRLF)
{
	// Arrange
	request.isChunked = true;

	// Act
	p.parseBody("6\r\nhello \r\n8\r\nw\r\norld!\r\n0\r\n\r\n", request, buffer);

	// Assert
	EXPECT_EQ(request.body, "hello w\r\norld!");
}

TEST_F(ParseBodyTest, NonChunkedBodySize14)
{
	// Arrange
	request.headers["content-length"] = "14";

	// Act
	p.parseBody("hello \r\nworld!", request, buffer);

	// Assert
	EXPECT_EQ(request.body, "hello \r\nworld!");
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
				EXPECT_STREQ(ERR_MISS_CRLF, e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(ParseBodyTest, IndicatedTooLargeChunkSize)
{
	// Arrange
	request.isChunked = true;

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.parseBody("35\r\nhello\r\n6\r\nworld!\r\n0\r\n\r\n", request, buffer);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ(ERR_CHUNKSIZE_INCONSISTENT, e.what());
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
				EXPECT_STREQ(ERR_MISS_CRLF, e.what());
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
				EXPECT_STREQ(ERR_MISS_CRLF, e.what());
				throw;
			}
		},
		std::runtime_error);
}
