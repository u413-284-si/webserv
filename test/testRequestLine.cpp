#include "RequestParser.hpp"
#include "gtest/gtest.h"

// VALID REQUEST LINE TEST SUITE

TEST(RequestParser_ValidRequestLine, BasicRequestLine_GET)
{
	RequestParser p;
	HTTPRequest request;

	p.parseHeader("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\n\r\n", request);
	EXPECT_EQ(request.method, MethodGet);
	EXPECT_EQ(request.uri.path, "/search");
	EXPECT_EQ(request.uri.query, "query=openai&year=2024");
	EXPECT_EQ(request.uri.fragment, "conclusion");
	EXPECT_EQ(request.version, "1.1");
}

TEST(RequestParser_ValidRequestLine, BasicRequestLine_DELETE)
{
	RequestParser p;
	HTTPRequest request;

	p.parseHeader("DELETE /index.html HTTP/1.1\r\n\r\n", request);
	EXPECT_EQ(request.method, MethodDelete);
	EXPECT_EQ(request.uri.path, "/index.html");
	EXPECT_EQ(request.uri.query, "");
	EXPECT_EQ(request.uri.fragment, "");
	EXPECT_EQ(request.version, "1.1");
}

TEST(RequestParser_ValidRequestLine, BasicRequestLine_POST)
{
	RequestParser p;
	HTTPRequest request;

	p.parseHeader("POST /abracadabra/ipsum?user=aziz&key=password HTTP/1.1\r\n\r\n", request);
	EXPECT_EQ(request.method, MethodPost);
	EXPECT_EQ(request.uri.path, "/abracadabra/ipsum");
	EXPECT_EQ(request.uri.query, "user=aziz&key=password");
	EXPECT_EQ(request.uri.fragment, "");
	EXPECT_EQ(request.version, "1.1");
}

TEST(RequestParser_ValidRequestLine, BasicRequestLine_NoQuery)
{
	RequestParser p;
	HTTPRequest request;

	p.parseHeader("GET /search? HTTP/1.1\r\n\r\n", request);
	EXPECT_EQ(request.method, MethodGet);
	EXPECT_EQ(request.uri.path, "/search");
	EXPECT_EQ(request.uri.query, "");
	EXPECT_EQ(request.uri.fragment, "");
	EXPECT_EQ(request.version, "1.1");
}

TEST(RequestParser_ValidRequestLine, BasicRequestLine_NoFragment)
{
	RequestParser p;
	HTTPRequest request;

	p.parseHeader("GET /search?# HTTP/1.1\r\n\r\n", request);
	EXPECT_EQ(request.method, MethodGet);
	EXPECT_EQ(request.uri.path, "/search");
	EXPECT_EQ(request.uri.query, "");
	EXPECT_EQ(request.uri.fragment, "");
	EXPECT_EQ(request.version, "1.1");
}

TEST(RequestParser_ValidRequestLine, Version1_0)
{
	RequestParser p;
	HTTPRequest request;

	p.parseHeader("GET /search?# HTTP/1.0\r\n\r\n", request);
	EXPECT_EQ(request.method, MethodGet);
	EXPECT_EQ(request.uri.path, "/search");
	EXPECT_EQ(request.uri.query, "");
	EXPECT_EQ(request.uri.fragment, "");
	EXPECT_EQ(request.version, "1.0");
}

// INVALID REQUEST LINE TEST SUITE

TEST(RequestParser_NonValidRequestLine, NotImplementedMethod)
{
	RequestParser p;
	HTTPRequest request;

	EXPECT_THROW(
		{
			try {
				p.parseHeader("PUT /search?query=openai&year=2024#conclusion HTTP/1.1\r\n\r\n", request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ("Invalid HTTP request: method not implemented", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST(RequestParser_NonValidRequestLine, LowerCaseMethod)
{
	RequestParser p;
	HTTPRequest request;

	EXPECT_THROW(
		{
			try {
				p.parseHeader("get /search?query=openai&year=2024#conclusion HTTP/1.1\r\n\r\n", request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ("Invalid HTTP request: method not implemented", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST(RequestParser_NonValidRequestLine, MissingSpace)
{
	RequestParser p;
	HTTPRequest request;

	EXPECT_THROW(
		{
			try {
				p.parseHeader("GET/search?query=openai&year=2024#conclusion HTTP/1.1\r\n\r\n", request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ("Invalid HTTP request: missing single space", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST(RequestParser_NonValidRequestLine, MissingSlash)
{
	RequestParser p;
	HTTPRequest request;

	EXPECT_THROW(
		{
			try {
				p.parseHeader("GET search?query=openai&year=2024#conclusion HTTP/1.1\r\n\r\n", request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ("Invalid HTTP request: missing slash in URI", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST(RequestParser_NonValidRequestLine, DoubleQuestionMark)
{
	RequestParser p;
	HTTPRequest request;

	EXPECT_THROW(
		{
			try {
				p.parseHeader("GET /search?? HTTP/1.1\r\n\r\n", request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ("Invalid HTTP request: invalid char in URI", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST(RequestParser_NonValidRequestLine, DoubleHash)
{
	RequestParser p;
	HTTPRequest request;

	EXPECT_THROW(
		{
			try {
				p.parseHeader("GET /search?## HTTP/1.1\r\n\r\n", request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ("Invalid HTTP request: invalid char in URI", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST(RequestParser_NonValidRequestLine, URI_InvalidChar)
{
	RequestParser p;
	HTTPRequest request;

	EXPECT_THROW(
		{
			try {
				p.parseHeader("GET /search§blabla/index.html HTTP/1.1\r\n\r\n", request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ("Invalid HTTP request: invalid char in URI", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST(RequestParser_NonValidRequestLine, URI_MissingSpace)
{
	RequestParser p;
	HTTPRequest request;

	EXPECT_THROW(
		{
			try {
				p.parseHeader("GET /searchHTTP/1.1\r\n\r\n", request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ("Invalid HTTP request: invalid char in URI", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST(RequestParser_NonValidRequestLine, Version_MissingH)
{
	RequestParser p;
	HTTPRequest request;

	EXPECT_THROW(
		{
			try {
				p.parseHeader("GET /search.html TTP/1.1\r\n\r\n", request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ("Invalid HTTP request: invalid format of version", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST(RequestParser_NonValidRequestLine, Version_MissingSlash)
{
	RequestParser p;
	HTTPRequest request;

	EXPECT_THROW(
		{
			try {
				p.parseHeader("GET /search.html HTTP1.1\r\n\r\n", request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ("Invalid HTTP request: invalid format of version", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST(RequestParser_NonValidRequestLine, Version_InvalidMajor)
{
	RequestParser p;
	HTTPRequest request;

	EXPECT_THROW(
		{
			try {
				p.parseHeader("GET /search.html HTTP/x.1\r\n\r\n", request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ("Invalid HTTP request: invalid version major", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST(RequestParser_NonValidRequestLine, Version_MissingDot)
{
	RequestParser p;
	HTTPRequest request;

	EXPECT_THROW(
		{
			try {
				p.parseHeader("GET /search.html HTTP/11\r\n\r\n", request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ("Invalid HTTP request: invalid version delimiter", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST(RequestParser_NonValidRequestLine, Version_InvalidMinor)
{
	RequestParser p;
	HTTPRequest request;

	EXPECT_THROW(
		{
			try {
				p.parseHeader("GET /search.html HTTP/1.x\r\n\r\n", request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ("Invalid HTTP request: invalid version minor", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST(RequestParser_NonValidRequestLine, Version_NonSupportedMajor)
{
	RequestParser p;
	HTTPRequest request;

	EXPECT_THROW(
		{
			try {
				p.parseHeader("GET /search.html HTTP/2.0\r\n\r\n", request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ("Invalid HTTP request: version not supported", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST(RequestParser_NonValidRequestLine, Version_NonSupportedMinor)
{
	RequestParser p;
	HTTPRequest request;

	EXPECT_THROW(
		{
			try {
				p.parseHeader("GET /search.html HTTP/1.3\r\n\r\n", request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ("Invalid HTTP request: version not supported", e.what());
				throw;
			}
		},
		std::runtime_error);
}
