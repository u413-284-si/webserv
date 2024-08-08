#include "gtest/gtest.h"
#include "RequestParser.hpp"

// VALID REQUEST LINE TEST SUITE

TEST(RequestParser_ValidRequestLine, BasicRequestLine_GET) {
	RequestParser	p;
	HTTPRequest		request;

	p.parseHttpRequest("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\n\r\n", request);
	EXPECT_EQ(request.method, MethodGet);
    EXPECT_EQ(request.uri.path , "/search");
    EXPECT_EQ(request.uri.query , "query=openai&year=2024");
    EXPECT_EQ(request.uri.fragment , "conclusion");
    EXPECT_EQ(request.version , "1.1");
}

TEST(RequestParser_ValidRequestLine, BasicRequestLine_DELETE) {
	RequestParser	p;
	HTTPRequest		request;

	p.parseHttpRequest("DELETE /index.html HTTP/1.1\r\n\r\n", request);
	EXPECT_EQ(request.method, MethodDelete);
    EXPECT_EQ(request.uri.path , "/index.html");
    EXPECT_EQ(request.uri.query , "");
    EXPECT_EQ(request.uri.fragment , "");
    EXPECT_EQ(request.version , "1.1");
}

TEST(RequestParser_ValidRequestLine, BasicRequestLine_POST) {
	RequestParser	p;
	HTTPRequest		request;

	p.parseHttpRequest("POST /abracadabra/ipsum?user=aziz&key=password HTTP/1.1\r\n\r\n", request);
	EXPECT_EQ(request.method, MethodPost);
    EXPECT_EQ(request.uri.path , "/abracadabra/ipsum");
    EXPECT_EQ(request.uri.query , "user=aziz&key=password");
    EXPECT_EQ(request.uri.fragment , "");
    EXPECT_EQ(request.version , "1.1");
}

TEST(RequestParser_ValidRequestLine, BasicRequestLine_NoQuery) {
	RequestParser	p;
	HTTPRequest		request;

	p.parseHttpRequest("GET /search? HTTP/1.1\r\n\r\n", request);
	EXPECT_EQ(request.method, MethodGet);
    EXPECT_EQ(request.uri.path , "/search");
    EXPECT_EQ(request.uri.query , "");
    EXPECT_EQ(request.uri.fragment , "");
    EXPECT_EQ(request.version , "1.1");
}

TEST(RequestParser_ValidRequestLine, BasicRequestLine_NoFragment) {
	RequestParser	p;
	HTTPRequest		request;

	p.parseHttpRequest("GET /search?# HTTP/1.1\r\n\r\n", request);
	EXPECT_EQ(request.method, MethodGet);
    EXPECT_EQ(request.uri.path , "/search");
    EXPECT_EQ(request.uri.query , "");
    EXPECT_EQ(request.uri.fragment , "");
    EXPECT_EQ(request.version , "1.1");
}

TEST(RequestParser_ValidRequestLine, Version1_0) {
	RequestParser	p;
	HTTPRequest		request;

	p.parseHttpRequest("GET /search?# HTTP/1.0\r\n\r\n", request);
	EXPECT_EQ(request.method, MethodGet);
    EXPECT_EQ(request.uri.path , "/search");
    EXPECT_EQ(request.uri.query , "");
    EXPECT_EQ(request.uri.fragment , "");
    EXPECT_EQ(request.version , "1.0");
}

// INVALID REQUEST LINE TEST SUITE

TEST(RequestParser_NonValidRequestLine, NotImplementedMethod) {
	RequestParser	p;
    HTTPRequest request;

	EXPECT_THROW(
		{
			try {
				p.parseHttpRequest("PUT /search?query=openai&year=2024#conclusion HTTP/1.1\r\n\r\n", request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ("Invalid HTTP request: method not implemented", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST(RequestParser_NonValidRequestLine, LowerCaseMethod) {
	RequestParser	p;
    HTTPRequest request;

	EXPECT_THROW(
		{
			try {
				p.parseHttpRequest("get /search?query=openai&year=2024#conclusion HTTP/1.1\r\n\r\n", request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ("Invalid HTTP request: method not implemented", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST(RequestParser_NonValidRequestLine, MissingSpace) {
	RequestParser	p;
    HTTPRequest request;

	EXPECT_THROW(
		{
			try {
				p.parseHttpRequest("GET/search?query=openai&year=2024#conclusion HTTP/1.1\r\n\r\n", request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ("Invalid HTTP request: missing single space", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST(RequestParser_NonValidRequestLine, MissingSlash) {
	RequestParser	p;
    HTTPRequest request;

	EXPECT_THROW(
		{
			try {
				p.parseHttpRequest("GET search?query=openai&year=2024#conclusion HTTP/1.1\r\n\r\n", request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ("Invalid HTTP request: missing slash in URI", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST(RequestParser_NonValidRequestLine, DoubleQuestionMark) {
	RequestParser	p;
    HTTPRequest request;

	EXPECT_THROW(
		{
			try {
				p.parseHttpRequest("GET /search?? HTTP/1.1\r\n\r\n", request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ("Invalid HTTP request: invalid char in URI", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST(RequestParser_NonValidRequestLine, DoubleHash) {
	RequestParser	p;
    HTTPRequest request;

	EXPECT_THROW(
		{
			try {
				p.parseHttpRequest("GET /search?## HTTP/1.1\r\n\r\n", request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ("Invalid HTTP request: invalid char in URI", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST(RequestParser_NonValidRequestLine, URI_InvalidChar) {
	RequestParser	p;
    HTTPRequest request;

	EXPECT_THROW(
		{
			try {
				p.parseHttpRequest("GET /search§blabla/index.html HTTP/1.1\r\n\r\n", request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ("Invalid HTTP request: invalid char in URI", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST(RequestParser_NonValidRequestLine, URI_MissingSpace) {
	RequestParser	p;
    HTTPRequest request;

	EXPECT_THROW(
		{
			try {
				p.parseHttpRequest("GET /searchHTTP/1.1\r\n\r\n", request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ("Invalid HTTP request: invalid char in URI", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST(RequestParser_NonValidRequestLine, Version_MissingH) {
	RequestParser	p;
    HTTPRequest request;

	EXPECT_THROW(
		{
			try {
				p.parseHttpRequest("GET /search.html TTP/1.1\r\n\r\n", request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ("Invalid HTTP request: invalid format of version", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST(RequestParser_NonValidRequestLine, Version_MissingSlash) {
	RequestParser	p;
    HTTPRequest request;

	EXPECT_THROW(
		{
			try {
				p.parseHttpRequest("GET /search.html HTTP1.1\r\n\r\n", request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ("Invalid HTTP request: invalid format of version", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST(RequestParser_NonValidRequestLine, Version_InvalidMajor) {
	RequestParser	p;
    HTTPRequest request;

	EXPECT_THROW(
		{
			try {
				p.parseHttpRequest("GET /search.html HTTP/x.1\r\n\r\n", request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ("Invalid HTTP request: invalid version major", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST(RequestParser_NonValidRequestLine, Version_MissingDot) {
	RequestParser	p;
    HTTPRequest request;

	EXPECT_THROW(
		{
			try {
				p.parseHttpRequest("GET /search.html HTTP/11\r\n\r\n", request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ("Invalid HTTP request: invalid version delimiter", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST(RequestParser_NonValidRequestLine, Version_InvalidMinor) {
	RequestParser	p;
    HTTPRequest request;

	EXPECT_THROW(
		{
			try {
				p.parseHttpRequest("GET /search.html HTTP/1.x\r\n\r\n", request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ("Invalid HTTP request: invalid version minor", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST(RequestParser_NonValidRequestLine, Version_NonSupportedMajor) {
	RequestParser	p;
    HTTPRequest request;

	EXPECT_THROW(
		{
			try {
				p.parseHttpRequest("GET /search.html HTTP/2.0\r\n\r\n", request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ("Invalid HTTP request: version not supported", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST(RequestParser_NonValidRequestLine, Version_NonSupportedMinor) {
	RequestParser	p;
    HTTPRequest request;

	EXPECT_THROW(
		{
			try {
				p.parseHttpRequest("GET /search.html HTTP/1.3\r\n\r\n", request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ("Invalid HTTP request: version not supported", e.what());
				throw;
			}
		},
		std::runtime_error);
}

// PREVIOUS TEST CODE

// void	runRequestLineTests(const std::string& name
// 		, size_t total
// 		, const std::pair<std::string, std::string> tests[])
// {
// 	std::cout << "\n* " << name << " *\n";
// 	for (size_t i = 0; i < total; i++) {
// 		std::cout << "\nTest [" << i << "]:\n" << tests[i].first << std::endl;
// 		try {
// 			RequestParser	p;
// 			HTTPRequest		request;
// 			std::string		result;

// 			request = p.parseHttpRequest(tests[i].first);
// 			std::istringstream	resultStream(tests[i].second);
			
// 			// Check method
// 			std::getline(resultStream, result);
// 			if (request.method == result)
// 				std::cout << "SUCCESS - Method: " << request.method << "\n";
// 			else {
// 				std::cerr << "FAILURE - expected: " << result << "\n";
// 				std::cerr << "received: " << request.method << "\n";
// 			}

// 			// Check path
// 			std::getline(resultStream, result);
// 			if (request.uri.path == result)
// 				std::cout << "SUCCESS - Path: " << request.uri.path << "\n";
// 			else {
// 				std::cerr << "FAILURE - expected: " << result << "\n";
// 				std::cerr << "received: " << request.uri.path << "\n";
// 			}

// 			// Check query
// 			std::getline(resultStream, result);
// 			if (request.uri.query == result)
// 				std::cout << "SUCCESS - Query: " << request.uri.query << "\n";
// 			else {
// 				std::cerr << "FAILURE - expected: " << result << "\n";
// 				std::cerr << "received: " << request.uri.query << "\n";
// 			}

// 			// Check fragment
// 			std::getline(resultStream, result);
// 			if (request.uri.fragment == result)
// 				std::cout << "SUCCESS - Fragment: " << request.uri.fragment << "\n";
// 			else {
// 				std::cerr << "FAILURE - expected: " << result << "\n";
// 				std::cerr << "received: " << request.uri.fragment << "\n";
// 			}

// 			// Check version
// 			std::getline(resultStream, result);
// 			if (request.version == result)
// 				std::cout << "SUCCESS - Version: " << request.version << "\n";
// 			else {
// 				std::cerr << "FAILURE - expected: " << result << "\n";
// 				std::cerr << "received: " << request.version << "\n";
// 			}
// 		}
// 		catch (std::exception& e) {
// 			std::cerr << "Error: " << e.what() << std::endl;
// 		}
// 	} 
// }

// void	testValidRequestLine()
// {
// 	std::pair<std::string, std::string>	tests[] = {
// 		std::make_pair("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\n\r\n"
// 			, "GET\n/search\nquery=openai&year=2024\nconclusion\n1.1"),
// 		std::make_pair("DELETE /index.html HTTP/1.1\r\n\r\n"
// 			, "DELETE\n/index.html\n\n\n1.1"),
// 		std::make_pair("POST /abracadabra/ipsum?user=aziz&key=password HTTP/2.0\r\n\r\n"
// 			, "POST\n/abracadabra/ipsum\nuser=aziz&key=password\n\n2.0"),
// 		std::make_pair("GET /search? HTTP/1.1\r\n\r\n"
// 			, "GET\n/search\n\n\n1.1"),
// 		std::make_pair("GET /search?# HTTP/1.1\r\n\r\n"
// 			, "GET\n/search\n\n\n1.1"),
// 	};
// 	runRequestLineTests("VALID REQUEST LINES", sizeof(tests) / sizeof(tests[0]), tests);
// }

// void	testInvalidRequestLine()
// {
// 	std::pair<std::string, std::string>	tests[] = {
// 		std::make_pair("PUT /search?query=openai&year=2024#conclusion HTTP/1.1\r\n"
// 			, "PUT\n/search\nquery=openai&year=2024\nconclusion\n1.1"),
// 		std::make_pair("get /search?query=openai&year=2024#conclusion HTTP/1.1\r\n"
// 			, "GET\n/search\nquery=openai&year=2024\nconclusion\n1.1"),
// 		std::make_pair("GET/search?query=openai&year=2024#conclusion HTTP/1.1\r\n"
// 			, "GET\n/search\nquery=openai&year=2024\nconclusion\n1.1"),
// 		std::make_pair("GET search?query=openai&year=2024#conclusion HTTP/1.1\r\n"
// 			, "GET\n/search\nquery=openai&year=2024\nconclusion\n1.1"),
// 		std::make_pair("GET /search?? HTTP/1.1\r\n"
// 			, "GET\n/search\n\n\n1.1"),
// 		std::make_pair("GET /search?## HTTP/1.1\r\n"
// 			, "GET\n/search\n\n\n1.1"),
// 		std::make_pair("GET /search§blabla/index.html HTTP/1.1\r\n"
// 			, "GET\n/search\n\n\n1.1"),	
// 		std::make_pair("GET /searchHTTP/1.1\r\n"
// 			, "GET\n/search\n\n\n1.1"),
// 		std::make_pair("GET /search.html TTP/1.1\r\n"
// 			, "GET\n/search.html\n\n\n1.1"),
// 		std::make_pair("GET /search.html HTTP1.1\r\n"
// 			, "GET\n/search.html\n\n\n1.1"),
// 		std::make_pair("GET /search.html HTTP/x.1\r\n"
// 			, "GET\n/search.html\n\n\n1.1"),
// 		std::make_pair("GET /search.html HTTP/11\r\n"
// 			, "GET\n/search.html\n\n\n1.1"),
// 		std::make_pair("GET /search.html HTTP/1.x\r\n"
// 			, "GET\n/search.html\n\n\n1.1"),
// 	};
// 	runRequestLineTests("INVALID REQUEST LINES", sizeof(tests) / sizeof(tests[0]), tests);
// }
