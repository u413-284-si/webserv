#include "RequestParser.hpp"
#include "error.hpp"
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
	EXPECT_EQ(request.headers["host"], "www.example.com");
	EXPECT_EQ(request.headers["user-agent"], "curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3");
	EXPECT_EQ(request.headers["accept-language"], "en, mi");
}

TEST_F(ParseHeadersTest, TrimWhiteSpaces)
{
	// Arrange

	// Act
	p.parseHeader(
		"GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost:       www.example.com       \r\n\r\n",
		request);

	// Assert
	EXPECT_EQ(request.headers["host"], "www.example.com");
}

TEST_F(ParseHeadersTest, ContentLength)
{
	// Arrange

	// Act
	p.parseHeader("POST /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: "
				  "www.example.com\r\nContent-Length: 23\r\n\r\n01234567890123456789012",
		request);

	// Assert
	EXPECT_EQ(request.headers["host"], "www.example.com");
	EXPECT_EQ(request.headers["content-length"], "23");
}

TEST_F(ParseHeadersTest, RepeatedEqualContentLength)
{
	// Arrange

	// Act
	p.parseHeader("POST /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: "
				  "www.example.com\r\nContent-Length: 23, 23\r\n\r\n01234567890123456789012",
		request);

	// Assert
	EXPECT_EQ(request.headers["host"], "www.example.com");
	EXPECT_EQ(request.headers["content-length"], "23");
}

TEST_F(ParseHeadersTest, TransferEncodingChunked)
{
	// Arrange

	// Act
	p.parseHeader("DELETE /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: "
				  "www.example.com\r\nTransfer-Encoding: gzip, chunked\r\n\r\n0\r\n\r\n",
		request);

	// Assert
	EXPECT_EQ(request.headers["host"], "www.example.com");
	EXPECT_EQ(request.headers["transfer-encoding"], "gzip, chunked");
}

TEST_F(ParseHeadersTest, TransferEncodingNonChunked)
{
	// Arrange

	// Act
	p.parseHeader("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: "
				  "www.example.com\r\nTransfer-Encoding: gzip\r\n\r\n",
		request);

	// Assert
	EXPECT_EQ(request.headers["host"], "www.example.com");
	EXPECT_EQ(request.headers["transfer-encoding"], "gzip");
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
	p.parseHeader(
		"GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: this.is.my.domain.com\r\n\r\n", request);

	// Assert
	EXPECT_EQ(request.headers["host"], "this.is.my.domain.com");
}

TEST_F(ParseHeadersTest, ValidHostnameAsIP)
{
	// Arrange

	// Act
	p.parseHeader("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n", request);

	// Assert
	EXPECT_EQ(request.headers["host"], "127.0.0.1");
}

TEST_F(ParseHeadersTest, ValidHostnameAsIPWithPort)
{
	// Arrange

	// Act
	p.parseHeader("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: 177.3.1.1:65535\r\n\r\n", request);

	// Assert
	EXPECT_EQ(request.headers["host"], "177.3.1.1:65535");
}

TEST_F(ParseHeadersTest, CloseConnection)
{
	// Arrange

	// Act
	p.parseHeader(
		"GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: 127.0.0.1\r\nConnection: close\r\n\r\n",
		request);

	// Assert
	EXPECT_EQ(request.headers["host"], "127.0.0.1");
	EXPECT_EQ(request.shallCloseConnection, true);
}

TEST_F(ParseHeadersTest, KeepConnection)
{
	// Arrange

	// Act
	p.parseHeader(
		"GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: 127.0.0.1\r\nConnection: keep-alive\r\n\r\n",
		request);

	// Assert
	EXPECT_EQ(request.headers["host"], "127.0.0.1");
	EXPECT_EQ(request.shallCloseConnection, false);
}

// NON-VALID HEADERS TEST SUITE

TEST_F(ParseHeadersTest, InvalidConnectionHeader)
{
	// Arrange

	// Act

	// Assert
	EXPECT_THROW(
		{
			try {
				p.parseHeader("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: "
							  "127.0.0.1\r\nConnection: doge\r\n\r\n",
					request);

			} catch (const std::runtime_error& e) {
				EXPECT_STREQ(ERR_INVALID_CONNECTION_VALUE, e.what());
				EXPECT_EQ(request.shallCloseConnection, true);
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(ParseHeadersTest, EmptyConnectionValue)
{
	// Arrange

	// Act

	// Assert
	EXPECT_THROW(
		{
			try {
				p.parseHeader("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: "
							  "127.0.0.1\r\nConnection: \r\n\r\n",
					request);

			} catch (const std::runtime_error& e) {
				EXPECT_STREQ(ERR_EMPTY_CONNECTION_VALUE, e.what());
				EXPECT_EQ(request.shallCloseConnection, true);
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(ParseHeadersTest, MultipleConnectionValues)
{
	// Arrange

	// Act

	// Assert
	EXPECT_THROW(
		{
			try {
				p.parseHeader("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: "
							  "127.0.0.1\r\nConnection: close, keep-alive, close\r\n\r\n",
					request);

			} catch (const std::runtime_error& e) {
				EXPECT_STREQ(ERR_MULTIPLE_CONNECTION_VALUES, e.what());
				EXPECT_EQ(request.shallCloseConnection, true);
				throw;
			}
		},
		std::runtime_error);
}

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
				EXPECT_STREQ(ERR_HEADER_COLON_WHITESPACE, e.what());
				EXPECT_EQ(request.shallCloseConnection, true);
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
				EXPECT_STREQ(ERR_OBSOLETE_LINE_FOLDING, e.what());
				EXPECT_EQ(request.shallCloseConnection, true);
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
				EXPECT_STREQ(ERR_HEADER_NAME_INVALID_CHAR, e.what());
				EXPECT_EQ(request.shallCloseConnection, true);
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
				EXPECT_STREQ(ERR_INVALID_CONTENT_LENGTH, e.what());
				EXPECT_EQ(request.shallCloseConnection, true);
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
				EXPECT_STREQ(ERR_INVALID_CONTENT_LENGTH, e.what());
				EXPECT_EQ(request.shallCloseConnection, true);
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
				EXPECT_STREQ(ERR_MULTIPLE_CONTENT_LENGTH_VALUES, e.what());
				EXPECT_EQ(request.shallCloseConnection, true);
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
				EXPECT_STREQ(ERR_INVALID_CONTENT_LENGTH, e.what());
				EXPECT_EQ(request.shallCloseConnection, true);
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
				EXPECT_STREQ(ERR_INVALID_CONTENT_LENGTH, e.what());
				EXPECT_EQ(request.shallCloseConnection, true);
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
				EXPECT_STREQ(ERR_MULTIPLE_CONTENT_LENGTH_VALUES, e.what());
				EXPECT_EQ(request.shallCloseConnection, true);
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
				EXPECT_STREQ(ERR_NON_EXISTENT_TRANSFER_ENCODING, e.what());
				EXPECT_EQ(request.shallCloseConnection, true);
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
				EXPECT_STREQ(ERR_NON_FINAL_CHUNKED_ENCODING, e.what());
				EXPECT_EQ(request.shallCloseConnection, true);
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
				EXPECT_STREQ(ERR_UNEXPECTED_BODY, e.what());
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
				p.parseHeader("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\n\r\n", request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ(ERR_MISSING_HOST_HEADER, e.what());
				EXPECT_EQ(request.shallCloseConnection, true);
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
				p.parseHeader("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost:\r\n\r\n", request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ(ERR_EMPTY_HOST_VALUE, e.what());
				EXPECT_EQ(request.shallCloseConnection, true);
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
				EXPECT_STREQ(ERR_MULTIPLE_HOST_HEADERS, e.what());
				EXPECT_EQ(request.shallCloseConnection, true);
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
				p.parseHeader(
					"GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: special-n@me.com\r\n\r\n",
					request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ(ERR_INVALID_HOSTNAME, e.what());
				EXPECT_EQ(request.shallCloseConnection, true);
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
				p.parseHeader(
					"GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: -invalid-hostname.com\r\n\r\n",
					request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ(ERR_INVALID_HOSTNAME, e.what());
				EXPECT_EQ(request.shallCloseConnection, true);
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
				p.parseHeader(
					"GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: invalid.-hostname.com\r\n\r\n",
					request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ(ERR_INVALID_HOSTNAME, e.what());
				EXPECT_EQ(request.shallCloseConnection, true);
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
				p.parseHeader(
					"GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: invalid-.hostname.com\r\n\r\n",
					request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ(ERR_INVALID_HOSTNAME, e.what());
				EXPECT_EQ(request.shallCloseConnection, true);
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
				p.parseHeader(
					"GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: invalid-hostname-.com\r\n\r\n",
					request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ(ERR_INVALID_HOSTNAME, e.what());
				EXPECT_EQ(request.shallCloseConnection, true);
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
				p.parseHeader("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: "
							  "this.is.a.too-long-label-"
							  "123456789012345678901234567890123456789012345678901234567890123.com\r\n\r\n",
					request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ(ERR_INVALID_HOSTNAME, e.what());
				EXPECT_EQ(request.shallCloseConnection, true);
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
				p.parseHeader("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: "
							  "this.is.a.too-long-hostname-"
							  "1234567890123456789012345678901234567890123456789012345678dfdfdfdfdfdfdfdfdfdferwert9012"
							  "3.com\r\n\r\n",
					request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ(ERR_INVALID_HOSTNAME, e.what());
				EXPECT_EQ(request.shallCloseConnection, true);
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
				p.parseHeader(
					"GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: 377.3.1.999\r\n\r\n", request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ(ERR_INVALID_HOST_IP, e.what());
				EXPECT_EQ(request.shallCloseConnection, true);
				throw;
			}
		},
		std::runtime_error);
}

// not an actual IP and missing alphabetical character for hostname
TEST_F(ParseHeadersTest, InvalidHostnameLikeIP)
{
	// Arrange

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.parseHeader(
					"GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: 377.3.1.9999\r\n\r\n", request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ(ERR_INVALID_HOST_IP, e.what());
				EXPECT_EQ(request.shallCloseConnection, true);
				throw;
			}
		},
		std::runtime_error);
}

// not an actual IP and missing alphabetical character for hostname
TEST_F(ParseHeadersTest, InvalidHostnameLikeIP2)
{
	// Arrange

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.parseHeader(
					"GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: 127.3.43.1.1\r\n\r\n", request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ(ERR_INVALID_HOST_IP, e.what());
				EXPECT_EQ(request.shallCloseConnection, true);
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(ParseHeadersTest, HostnameAsIPWithInvalidPort)
{
	// Arrange

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.parseHeader(
					"GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: 177.3.1.1:65536\r\n\r\n", request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ(ERR_INVALID_HOST_IP_WITH_PORT, e.what());
				EXPECT_EQ(request.shallCloseConnection, true);
				throw;
			}
		},
		std::runtime_error);
}
