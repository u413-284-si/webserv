#include "gtest/gtest.h"
#include "RequestParser.hpp"

// VALID BODY TEST SUITE

TEST(RequestParser_ValidBody, ChunkedBody) {
	RequestParser	p;
	HTTPRequest		request;

	p.parseHttpRequest("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nTransfer-Encoding: gzip, chunked\r\n\r\n6\r\nhello \r\n6\r\nworld!\r\n0\r\n\r\n", request);
	EXPECT_EQ(request.body, "hello world!");
}

TEST(RequestParser_ValidBody, NoBodyTrigger) {
	RequestParser	p;
	HTTPRequest		request;

	p.parseHttpRequest("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nTransfer-Encoding: gzip\r\n\r\nhello \r\nworld!\r\n", request);
	EXPECT_EQ(request.body, "");
}

TEST(RequestParser_ValidBody, NonChunkedBody) {
	RequestParser	p;
	HTTPRequest		request;

	p.parseHttpRequest("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: 14\r\n\r\nhello \r\nworld!", request);
	EXPECT_EQ(request.body, "hello \nworld!");
    p.clearRequest(request);
	p.clearParser();
	p.parseHttpRequest("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: 16\r\n\r\nhello \r\nworld!\r\n", request);
	EXPECT_EQ(request.body, "hello \nworld!\n");
}

// INVALID BODY TEST SUITE

TEST(RequestParser_NonValidBody, DifferingChunkSize) {
	RequestParser	p;
    HTTPRequest request;

	EXPECT_THROW(p.parseHttpRequest("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nTransfer-Encoding: gzip, chunked\r\n\r\n1\r\nhello \r\n6\r\nworld!\r\n0\r\n\r\n", request), std::runtime_error);
    p.clearRequest(request);
    p.clearParser();
    try{
        p.parseHttpRequest("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nTransfer-Encoding: gzip, chunked\r\n\r\n1\r\nhello \r\n6\r\nworld!\r\n0\r\n\r\n", request);
    }
    catch(const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
    }
}

TEST(RequestParser_NonValidBody, DifferingContentLength) {
	RequestParser	p;
    HTTPRequest request;

	EXPECT_THROW(p.parseHttpRequest("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: 3\r\n\r\nhello \r\nworld!\r\n", request), std::runtime_error);
    p.clearRequest(request);
    p.clearParser();
    try{
        p.parseHttpRequest("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: 3\r\n\r\nhello \r\nworld!\r\n", request);
    }
    catch(const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
    }
}

TEST(RequestParser_NonValidBody, MissingCRLFInChunk) {
	RequestParser	p;
    HTTPRequest request;

	EXPECT_THROW(p.parseHttpRequest("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nTransfer-Encoding: gzip, chunked\r\n\r\n6\r\nhello 6\r\nworld!\r\n0\r\n\r\n", request), std::runtime_error);
    p.clearRequest(request);
    p.clearParser();
    try{
        p.parseHttpRequest("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nTransfer-Encoding: gzip, chunked\r\n\r\n6\r\nhello 6\r\nworld!\r\n0\r\n\r\n", request);
    }
    catch(const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
    }
}

TEST(RequestParser_NonValidBody, MissingCRInChunk) {
	RequestParser	p;
    HTTPRequest request;

	EXPECT_THROW(p.parseHttpRequest("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nTransfer-Encoding: gzip, chunked\r\n\r\n6\r\nhello \n6\r\nworld!\r\n0\r\n\r\n", request), std::runtime_error);
    p.clearRequest(request);
    p.clearParser();
    try{
        p.parseHttpRequest("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nTransfer-Encoding: gzip, chunked\r\n\r\n6\r\nhello \n6\r\nworld!\r\n0\r\n\r\n", request);
    }
    catch(const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
    }
}

// PREVIOUS TEST CODE

// void	runBodyTests(const std::string& name
// 		, size_t total
// 		, const std::pair<std::string, std::string> tests[])
// {
// 	std::cout << "\n* " << name << " *\n";
// 	for (size_t i = 0; i < total; i++) {
// 		std::cout << "\nTest [" << i << "]:\n" << tests[i].first << std::endl;
// 		try {
// 			RequestParser	p;
// 			HTTPRequest		request;
// 			std::string		body;

// 			request = p.parseHttpRequest(tests[i].first);
			
// 			// Check body string
// 			if (request.body == tests[i].second)
// 				std::cout << "SUCCESS - Body: " << request.body << "\n";
// 			else {
// 				std::cerr << "FAILURE - expected: " << tests[i].second << "\n";
// 				std::cerr << "received: " << request.body << "\n";
// 		}
// 		}
// 		catch (std::exception& e) {
// 			std::cerr << "Error: " << e.what() << std::endl;
// 		}
// 	} 
// }

// void	testValidBody()
// {
// 	std::pair<std::string, std::string>	tests[] = {
// 		std::make_pair("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nTransfer-Encoding: gzip, chunked\r\n\r\n6\r\nhello \r\n6\r\nworld!\r\n0\r\n\r\n"
// 			, "hello world!"),
// 		std::make_pair("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nTransfer-Encoding: gzip\r\n\r\nhello \r\nworld!\r\n"
// 			, ""),
// 		std::make_pair("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: 14\r\n\r\nhello \r\nworld!"
// 			, "hello \nworld!"),
// 		std::make_pair("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: 16\r\n\r\nhello \r\nworld!\r\n"
// 			, "hello \nworld!\n"),

// 	};
// 	runBodyTests("VALID BODY", sizeof(tests) / sizeof(tests[0]), tests);
// }

// void	testInvalidBody()
// {
// 	std::pair<std::string, std::string>	tests[] = {
// 		std::make_pair("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nTransfer-Encoding: gzip, chunked\r\n\r\n1\r\nhello \r\n6\r\nworld!\r\n0\r\n\r\n"
// 			, "hello world!"),
// 		std::make_pair("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: 3\r\n\r\nhello \r\nworld!\r\n"
// 			, "hello \nworld!"),
// 		std::make_pair("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nTransfer-Encoding: gzip, chunked\r\n\r\n6\r\nhello 6\r\nworld!\r\n0\r\n\r\n"
// 			, "hello world!"),
// 		std::make_pair("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nTransfer-Encoding: gzip, chunked\r\n\r\n6\r\nhello \n6\r\nworld!\r\n0\r\n\r\n"
// 			, "hello world!"),
// 	};
// 	runBodyTests("INVALID BODY", sizeof(tests) / sizeof(tests[0]), tests);
// }
