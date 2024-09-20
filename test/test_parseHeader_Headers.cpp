#include "RequestParser.hpp"
#include "gtest/gtest.h"
#include <stdexcept>

class ParseHeadersTest : public ::testing::Test {
protected:
	ParseHeadersTest()
		: request()
	{
		request.hasBody = false;
		request.isChunked = false;
	}
	~ParseHeadersTest() override { }

	RequestParser p;
	HTTPRequest request;
};

// VALID HEADER TEST SUITE

TEST_F(ParseHeadersTest, ValidHeaders)
{
	// Arrange

	// Act
	p.parseHeader("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nUser-Agent: "
				  "curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3\r\nAccept-Language: en, mi\r\n\r\n",
		request);

	// Assert
	EXPECT_EQ(request.headers["Host"], "www.example.com");
	EXPECT_EQ(request.headers["User-Agent"], "curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3");
	EXPECT_EQ(request.headers["Accept-Language"], "en, mi");
}

TEST_F(ParseHeadersTest, TrimWhiteSpaces)
{
	// Arrange

	// Act
	p.parseHeader(
		"GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost:       www.example.com       \r\n\r\n",
		request);

	// Assert
	EXPECT_EQ(request.headers["Host"], "www.example.com");
}

TEST_F(ParseHeadersTest, ContentLength)
{
	// Arrange

	// Act
	p.parseHeader("POST /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: "
				  "www.example.com\r\nContent-Length: 23\r\n\r\n01234567890123456789012",
		request);

	// Assert
	EXPECT_EQ(request.headers["Host"], "www.example.com");
	EXPECT_EQ(request.headers["Content-Length"], "23");
}

TEST_F(ParseHeadersTest, RepeatedEqualContentLength)
{
	// Arrange

	// Act
	p.parseHeader("POST /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: "
				  "www.example.com\r\nContent-Length: 23, 23\r\n\r\n01234567890123456789012",
		request);

	// Assert
	EXPECT_EQ(request.headers["Host"], "www.example.com");
	EXPECT_EQ(request.headers["Content-Length"], "23");
}

TEST_F(ParseHeadersTest, TransferEncodingChunked)
{
	// Arrange

	// Act
	p.parseHeader("DELETE /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: "
				  "www.example.com\r\nTransfer-Encoding: gzip, chunked\r\n\r\n0\r\n\r\n",
		request);

	// Assert
	EXPECT_EQ(request.headers["Host"], "www.example.com");
	EXPECT_EQ(request.headers["Transfer-Encoding"], "gzip, chunked");
}

TEST_F(ParseHeadersTest, TransferEncodingNonChunked)
{
	// Arrange

	// Act
	p.parseHeader("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: "
				  "www.example.com\r\nTransfer-Encoding: gzip\r\n\r\n",
		request);

	// Assert
	EXPECT_EQ(request.headers["Host"], "www.example.com");
	EXPECT_EQ(request.headers["Transfer-Encoding"], "gzip");
}

TEST_F(ParseHeadersTest, NoBodyTrigger)
{
	// Arrange

	// Act
	p.parseHeader("DELETE /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: "
				  "www.example.com\r\nTransfer-Encoding: gzip\r\n\r\nhello \r\nworld!\r\n",
		request);

	// Assert
	EXPECT_FALSE(request.hasBody);
}

TEST_F(ParseHeadersTest, ValidHostname)
{
	// Arrange

	// Act
	p.parseHeader("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: this.is.my.domain.com\r\n\r\n",
					request);

	// Assert
	EXPECT_EQ(request.headers["Host"], "this.is.my.domain.com");
}

TEST_F(ParseHeadersTest, ValidHostnameAsIP)
{
	// Arrange

	// Act
	p.parseHeader("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n",
					request);

	// Assert
	EXPECT_EQ(request.headers["Host"], "127.0.0.1");
}


// NON-VALID HEADERS TEST SUITE

TEST_F(ParseHeadersTest, WhitespaceBetweenHeaderFieldNameAndColon)
{
	// Arrange

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.parseHeader("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost :       "
							  "www.example.com       \r\n\r\n",
					request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ("Invalid HTTP request: Whitespace between header field-name and colon detected", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(ParseHeadersTest, ObsoleteLineFolding)
{
	// Arrange

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.parseHeader("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\n Host:       "
							  "www.example.com       \r\n\r\n",
					request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ("Invalid HTTP request: Obsolete line folding detected", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(ParseHeadersTest, InvalidFieldName)
{
	// Arrange

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.parseHeader(
					"GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nH<ost: www.example.com\r\n\r\n",
					request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ("Invalid HTTP request: Invalid char in header field name", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(ParseHeadersTest, EmptyContentLength)
{
	// Arrange

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.parseHeader("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: "
							  "www.example.com\r\nContent-Length: \r\n\r\n",
					request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ("Invalid HTTP request: Invalid content-length provided", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(ParseHeadersTest, InvalidContentLength)
{
	// Arrange

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.parseHeader("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: "
							  "www.example.com\r\nContent-Length: ur\r\n\r\n",
					request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ("Invalid HTTP request: Invalid content-length provided", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(ParseHeadersTest, RepeatedNonEqualContentLengths)
{
	// Arrange

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.parseHeader("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: "
							  "www.example.com\r\nContent-Length: 23, 1\r\n\r\n",
					request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ("Invalid HTTP request: Multiple differing content-length values", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(ParseHeadersTest, InvalidInFirstRepeatedContentLength)
{
	// Arrange

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.parseHeader("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: "
							  "www.example.com\r\nContent-Length: 23s, 23\r\n\r\n",
					request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ("Invalid HTTP request: Invalid content-length provided", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(ParseHeadersTest, InvalidInLastRepeatedContentLength)
{
	// Arrange

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.parseHeader("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: "
							  "www.example.com\r\nContent-Length: 23, e23\r\n\r\n",
					request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ("Invalid HTTP request: Invalid content-length provided", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(ParseHeadersTest, RepeatedContentLengthHeaders)
{
	// Arrange

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.parseHeader("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: "
							  "www.example.com\r\nContent-Length: 23\r\nContent-Length: 2\r\n\r\n",
					request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ("Invalid HTTP request: Multiple differing content-length values", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(ParseHeadersTest, EmptyTransferEncoding)
{
	// Arrange

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.parseHeader("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: "
							  "www.example.com\r\nTransfer-Encoding: \r\n\r\n",
					request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ("Invalid HTTP request: Transfer encoding not detected", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(ParseHeadersTest, InvalidChunkedTransferEncodingPositioning)
{
	// Arrange

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.parseHeader("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: "
							  "www.example.com\r\nTransfer-Encoding: chunked, gzip\r\n\r\n",
					request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ("Invalid HTTP request: Chunked encoding not the final encoding", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(ParseHeadersTest, UnexpectedBody)
{
	// Arrange

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.parseHeader("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: "
							  "www.example.com\r\nTransfer-Encoding: gzip, chunked\r\n\r\n0\r\n\r\n",
					request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ("Invalid HTTP request: Method should not have a body", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(ParseHeadersTest, MissingHostHeader)
{
	// Arrange

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.parseHeader("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\n\r\n",
					request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ("Invalid HTTP request: Missing host header", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(ParseHeadersTest, EmptyHostValue)
{
	// Arrange

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.parseHeader("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost:\r\n\r\n",
					request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ("Invalid HTTP request: Missing hostname", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(ParseHeadersTest, MultipleHostHeaders)
{
	// Arrange

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.parseHeader("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: example.com\r\n"
					"SomeOtherHeader: random value\r\nHost: huhu.com\r\n\r\n",
					request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ("Invalid HTTP request: Multiple host headers", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(ParseHeadersTest, HostnameInvalidChar)
{
	// Arrange

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.parseHeader("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: special-n@me.com\r\n\r\n",
					request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ("Invalid HTTP request: Invalid hostname", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(ParseHeadersTest, HostnameInvalidHyphenAtStart)
{
	// Arrange

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.parseHeader("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: -invalid-hostname.com\r\n\r\n",
					request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ("Invalid HTTP request: Invalid hostname", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(ParseHeadersTest, HostnameInvalidHyphenAtStartOfLabel)
{
	// Arrange

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.parseHeader("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: invalid.-hostname.com\r\n\r\n",
					request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ("Invalid HTTP request: Invalid hostname", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(ParseHeadersTest, HostnameInvalidHyphenAtEndOfLabel)
{
	// Arrange

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.parseHeader("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: invalid-.hostname.com\r\n\r\n",
					request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ("Invalid HTTP request: Invalid hostname", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(ParseHeadersTest, HostnameInvalidHyphenAtEnd)
{
	// Arrange

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.parseHeader("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: invalid-hostname-.com\r\n\r\n",
					request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ("Invalid HTTP request: Invalid hostname", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(ParseHeadersTest, HostnameLabelTooLong)
{
	// Arrange

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.parseHeader("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: this.is.a.too-long-label-123456789012345678901234567890123456789012345678901234567890123.com\r\n\r\n",
					request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ("Invalid HTTP request: Invalid hostname", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(ParseHeadersTest, HostnameTooLong)
{
	// Arrange

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.parseHeader("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: this.is.a.too-long-hostname-1234567890123456789012345678901234567890123456789012345678dfdfdfdfdfdfdfdfdfdferwert90123.com\r\n\r\n",
					request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ("Invalid HTTP request: Invalid hostname", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(ParseHeadersTest, HostnameAsIPInvalid)
{
	// Arrange

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.parseHeader("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: 377.3.1.9999\r\n\r\n",
					request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ("Invalid HTTP request: Invalid hostname", e.what());
				throw;
			}
		},
		std::runtime_error);
}
