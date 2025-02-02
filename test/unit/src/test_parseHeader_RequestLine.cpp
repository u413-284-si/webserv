#include "RequestParser.hpp"
#include "gtest/gtest.h"

class ParseRequestLineTest : public ::testing::Test {
protected:
	ParseRequestLineTest()
		: request()
	{
		request.hasBody = false;
		request.isChunked = false;
	}
	~ParseRequestLineTest() override { }

	RequestParser p;
	HTTPRequest request;
};

// VALID REQUEST LINE TEST SUITE

TEST_F(ParseRequestLineTest, BasicRequestLine_GET)
{
	// Arrange

	// Act
	p.parseHeader("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\n\r\n", request);

	// Assert
	EXPECT_EQ(request.method, MethodGet);
	EXPECT_EQ(request.uri.path, "/search");
	EXPECT_EQ(request.uri.query, "query=openai&year=2024");
	EXPECT_EQ(request.uri.fragment, "conclusion");
	EXPECT_EQ(request.version, "1.1");
}

TEST_F(ParseRequestLineTest, BasicRequestLine_DELETE)
{
	// Arrange

	// Act
	p.parseHeader("DELETE /index.html HTTP/1.1\r\nHost: www.example.com\r\n\r\n", request);

	// Assert
	EXPECT_EQ(request.method, MethodDelete);
	EXPECT_EQ(request.uri.path, "/index.html");
	EXPECT_EQ(request.uri.query, "");
	EXPECT_EQ(request.uri.fragment, "");
	EXPECT_EQ(request.version, "1.1");
}

TEST_F(ParseRequestLineTest, BasicRequestLine_POST)
{
	// Arrange

	// Act
	p.parseHeader("POST /abracadabra/ipsum?user=aziz&key=password HTTP/1.1\r\nHost: www.example.com\r\n\r\n", request);

	// Assert
	EXPECT_EQ(request.method, MethodPost);
	EXPECT_EQ(request.uri.path, "/abracadabra/ipsum");
	EXPECT_EQ(request.uri.query, "user=aziz&key=password");
	EXPECT_EQ(request.uri.fragment, "");
	EXPECT_EQ(request.version, "1.1");
}

TEST_F(ParseRequestLineTest, BasicRequestLine_NoQuery)
{
	// Arrange

	// Act
	p.parseHeader("GET /search? HTTP/1.1\r\nHost: www.example.com\r\n\r\n", request);

	// Assert
	EXPECT_EQ(request.method, MethodGet);
	EXPECT_EQ(request.uri.path, "/search");
	EXPECT_EQ(request.uri.query, "");
	EXPECT_EQ(request.uri.fragment, "");
	EXPECT_EQ(request.version, "1.1");
}

TEST_F(ParseRequestLineTest, BasicRequestLine_NoFragment)
{
	// Arrange

	// Act
	p.parseHeader("GET /search?# HTTP/1.1\r\nHost: www.example.com\r\n\r\n", request);

	// Assert
	EXPECT_EQ(request.method, MethodGet);
	EXPECT_EQ(request.uri.path, "/search");
	EXPECT_EQ(request.uri.query, "");
	EXPECT_EQ(request.uri.fragment, "");
	EXPECT_EQ(request.version, "1.1");
}

TEST_F(ParseRequestLineTest, RequestLinePercentEncoded)
{
	// Arrange

	// Act
	p.parseHeader("GET /search%20maschine?query=%C3%B6sterreich&dialekt=%23#%F0%9F%98%8A HTTP/1.1\r\nHost: www.example.com\r\n\r\n", request);

	// Assert
	EXPECT_EQ(request.method, MethodGet);
	EXPECT_EQ(request.uri.path, "/search maschine");
	EXPECT_EQ(request.uri.query, "query=österreich&dialekt=#");
	EXPECT_EQ(request.uri.fragment, "😊");
	EXPECT_EQ(request.version, "1.1");
}

TEST_F(ParseRequestLineTest, RequestLinePercentEncodedInvalidNUL)
{
	// Arrange

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.parseHeader("GET /search%00 HTTP/1.1\r\nHost: www.example.com\r\n\r\n", request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ(ERR_PERCENT_NONSUPPORTED_NUL, e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(ParseRequestLineTest, RequestLinePercentEncodedNotComplete)
{
	// Arrange

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.parseHeader("GET /search%F HTTP/1.1\r\nHost: www.example.com\r\n\r\n", request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ(ERR_PERCENT_INCOMPLETE, e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(ParseRequestLineTest, RequestLinePercentEncodedNonHex)
{
	// Arrange

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.parseHeader("GET /search%4& HTTP/1.1\r\nHost: www.example.com\r\n\r\n", request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ(ERR_PERCENT_INVALID_HEX, e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(ParseRequestLineTest, RequestLineWithDotSegments)
{
	// Arrange

	// Act
	p.parseHeader("GET /search/.././hello/ HTTP/1.1\r\nHost: www.example.com\r\n\r\n", request);

	// Assert
	EXPECT_EQ(request.method, MethodGet);
	EXPECT_EQ(request.uri.path, "/hello/");
	EXPECT_EQ(request.version, "1.1");
}

TEST_F(ParseRequestLineTest, RequestLineWithDotSegmentsOnlySlashLeft)
{
	// Arrange

	// Act
	p.parseHeader("GET /search/.. HTTP/1.1\r\nHost: www.example.com\r\n\r\n", request);

	// Assert
	EXPECT_EQ(request.method, MethodGet);
	EXPECT_EQ(request.uri.path, "/");
	EXPECT_EQ(request.version, "1.1");
}

TEST_F(ParseRequestLineTest, RequestLineWithDotsInName)
{
	// Arrange

	// Act
	p.parseHeader("GET /search../ HTTP/1.1\r\nHost: www.example.com\r\n\r\n", request);

	// Assert
	EXPECT_EQ(request.method, MethodGet);
	EXPECT_EQ(request.uri.path, "/search../");
	EXPECT_EQ(request.version, "1.1");
}

TEST_F(ParseRequestLineTest, RequestLineOnlyDoubleDots)
{
	// Arrange

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.parseHeader("GET /.. HTTP/1.1\r\nHost: www.example.com\r\n\r\n", request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ(ERR_DIRECTORY_TRAVERSAL, e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(ParseRequestLineTest, RequestLineWithTooManyDotSegments)
{
	// Arrange

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.parseHeader("GET /search/../../hello HTTP/1.1\r\nHost: www.example.com\r\n\r\n", request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ(ERR_DIRECTORY_TRAVERSAL, e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(ParseRequestLineTest, Version1_0)
{
	// Arrange

	// Act
	p.parseHeader("GET /search?# HTTP/1.0\r\nHost: www.example.com\r\n\r\n", request);

	// Assert
	EXPECT_EQ(request.method, MethodGet);
	EXPECT_EQ(request.uri.path, "/search");
	EXPECT_EQ(request.uri.query, "");
	EXPECT_EQ(request.uri.fragment, "");
	EXPECT_EQ(request.version, "1.0");
}

// INVALID REQUEST LINE TEST SUITE

TEST_F(ParseRequestLineTest, NotImplementedMethod)
{
	// Arrange

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.parseHeader("PUT /search?query=openai&year=2024#conclusion HTTP/1.1\r\n\r\n", request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ(ERR_METHOD_NOT_IMPLEMENTED, e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(ParseRequestLineTest, LowerCaseMethod)
{
	// Arrange

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.parseHeader("get /search?query=openai&year=2024#conclusion HTTP/1.1\r\n\r\n", request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ(ERR_METHOD_NOT_IMPLEMENTED, e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(ParseRequestLineTest, MissingSpace)
{
	// Arrange

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.parseHeader("GET/search?query=openai&year=2024#conclusion HTTP/1.1\r\n\r\n", request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ(ERR_MISS_SINGLE_SPACE, e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(ParseRequestLineTest, MissingSlash)
{
	// Arrange

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.parseHeader("GET search?query=openai&year=2024#conclusion HTTP/1.1\r\n\r\n", request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ(ERR_URI_MISS_SLASH, e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(ParseRequestLineTest, DoubleQuestionMark)
{
	// Arrange

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.parseHeader("GET /search?? HTTP/1.1\r\n\r\n", request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ(ERR_URI_INVALID_CHAR, e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(ParseRequestLineTest, DoubleHash)
{
	// Arrange

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.parseHeader("GET /search?## HTTP/1.1\r\n\r\n", request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ(ERR_URI_INVALID_CHAR, e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(ParseRequestLineTest, URI_InvalidChar)
{
	// Arrange

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.parseHeader("GET /search§blabla/index.html HTTP/1.1\r\n\r\n", request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ(ERR_URI_INVALID_CHAR, e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(ParseRequestLineTest, URI_MissingSpace)
{
	// Arrange

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.parseHeader("GET /searchHTTP/1.1\r\n\r\n", request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ(ERR_URI_INVALID_CHAR, e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(ParseRequestLineTest, Version_MissingH)
{
	// Arrange

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.parseHeader("GET /search.html TTP/1.1\r\n\r\n", request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ(ERR_INVALID_VERSION_FORMAT, e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(ParseRequestLineTest, Version_MissingSlash)
{
	// Arrange

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.parseHeader("GET /search.html HTTP1.1\r\n\r\n", request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ(ERR_INVALID_VERSION_FORMAT, e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(ParseRequestLineTest, Version_InvalidMajor)
{
	// Arrange

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.parseHeader("GET /search.html HTTP/x.1\r\n\r\n", request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ(ERR_INVALID_VERSION_MAJOR, e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(ParseRequestLineTest, Version_MissingDot)
{
	// Arrange

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.parseHeader("GET /search.html HTTP/11\r\n\r\n", request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ(ERR_INVALID_VERSION_DELIM, e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(ParseRequestLineTest, Version_InvalidMinor)
{
	// Arrange

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.parseHeader("GET /search.html HTTP/1.x\r\n\r\n", request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ(ERR_INVALID_VERSION_MINOR, e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(ParseRequestLineTest, Version_NonSupportedMajor)
{
	// Arrange

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.parseHeader("GET /search.html HTTP/2.0\r\n\r\n", request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ(ERR_NONSUPPORTED_VERSION, e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(ParseRequestLineTest, Version_NonSupportedMinor)
{
	// Arrange

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.parseHeader("GET /search.html HTTP/1.3\r\n\r\n", request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ(ERR_NONSUPPORTED_VERSION, e.what());
				throw;
			}
		},
		std::runtime_error);
}
