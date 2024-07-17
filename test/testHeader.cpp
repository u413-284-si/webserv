#include "gtest/gtest.h"
#include <stdexcept>
#include "RequestParser.hpp"


// VALID HEADER TEST SUITE

TEST(RequestParser_ValidHeaders, ValidHeaders) {
	RequestParser	p;
	HTTPRequest		request;

	request = p.parseHttpRequest("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nUser-Agent: curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3\r\nAccept-Language: en, mi\r\n\r\n");
	EXPECT_EQ(request.headers["Host"], "www.example.com");
    EXPECT_EQ(request.headers["User-Agent"], "curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3");
    EXPECT_EQ(request.headers["Accept-Language"], "en, mi");
}

TEST(RequestParser_ValidHeaders, TrimWhiteSpaces) {
	RequestParser	p;
	HTTPRequest		request;

	request = p.parseHttpRequest("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost:       www.example.com       \r\n\r\n");
	EXPECT_EQ(request.headers["Host"], "www.example.com");
}

TEST(RequestParser_ValidHeaders, ContentLength) {
	RequestParser	p;
	HTTPRequest		request;

	request = p.parseHttpRequest("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: 23\r\n\r\n01234567890123456789012");
	EXPECT_EQ(request.headers["Host"], "www.example.com");
	EXPECT_EQ(request.headers["Content-Length"], "23");
}

TEST(RequestParser_ValidHeaders, RepeatedEqualContentLength) {
	RequestParser	p;
	HTTPRequest		request;

	request = p.parseHttpRequest("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: 23, 23\r\n\r\n01234567890123456789012");
	EXPECT_EQ(request.headers["Host"], "www.example.com");
	EXPECT_EQ(request.headers["Content-Length"], "23");
}

TEST(RequestParser_ValidHeaders, TransferEncodingChunked) {
	RequestParser	p;
	HTTPRequest		request;

	request = p.parseHttpRequest("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nTransfer-Encoding: gzip, chunked\r\n\r\n0\r\n\r\n");
	EXPECT_EQ(request.headers["Host"], "www.example.com");
	EXPECT_EQ(request.headers["Transfer-Encoding"], "gzip, chunked");
}

TEST(RequestParser_ValidHeaders, TransferEncodingNonChunked) {
	RequestParser	p;
	HTTPRequest		request;

	request = p.parseHttpRequest("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nTransfer-Encoding: gzip\r\n\r\n");
	EXPECT_EQ(request.headers["Host"], "www.example.com");
	EXPECT_EQ(request.headers["Transfer-Encoding"], "gzip");
}

// NON-VALID HEADERS TEST SUITE

TEST(RequestParser_NonValidHeaders, WhitespaceBetweenHeaderFieldNameAndColon) {
	RequestParser	p;

	EXPECT_THROW(p.parseHttpRequest("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost :       www.example.com       \r\n\r\n"), std::runtime_error);
    p.clearParser();
    try{
        p.parseHttpRequest("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost :       www.example.com       \r\n\r\n");
    }
    catch(const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
    }
}

TEST(RequestParser_NonValidHeaders, ObsoleteLineFolding) {
	RequestParser	p;

	EXPECT_THROW(p.parseHttpRequest("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\n Host:       www.example.com       \r\n\r\n"), std::runtime_error);
    p.clearParser();
    try{
        p.parseHttpRequest("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\n Host:       www.example.com       \r\n\r\n");
    }
    catch(const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
    }
}

TEST(RequestParser_NonValidHeaders, InvalidFieldName) {
	RequestParser	p;

	EXPECT_THROW(p.parseHttpRequest("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nH<ost: www.example.com\r\n\r\n"), std::runtime_error);
    p.clearParser();
    try{
        p.parseHttpRequest("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nH<ost: www.example.com\r\n\r\n");
    }
    catch(const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
    }
}

TEST(RequestParser_NonValidHeaders, MissingCRLFAfterHeaderSection) {
	RequestParser	p;

	EXPECT_THROW(p.parseHttpRequest("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\n"), std::runtime_error);
    p.clearParser();
    try{
        p.parseHttpRequest("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\n");
    }
    catch(const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
    }
}

TEST(RequestParser_NonValidHeaders, EmptyContentLength) {
	RequestParser	p;

	EXPECT_THROW(p.parseHttpRequest("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: \r\n\r\n"), std::runtime_error);
    p.clearParser();
    try{
        p.parseHttpRequest("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: \r\n\r\n");
    }
    catch(const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
    }
}

TEST(RequestParser_NonValidHeaders, InvalidContentLength) {
	RequestParser	p;

	EXPECT_THROW(p.parseHttpRequest("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: ur\r\n\r\n"), std::runtime_error);
    p.clearParser();
    try{
        p.parseHttpRequest("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: ur\r\n\r\n");
    }
    catch(const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
    }
}

TEST(RequestParser_NonValidHeaders, RepeatedNonEqualContentLengths) {
	RequestParser	p;

	EXPECT_THROW(p.parseHttpRequest("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: 23, 1\r\n\r\n"), std::runtime_error);
    p.clearParser();
    try{
        p.parseHttpRequest("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: 23, 1\r\n\r\n");
    }
    catch(const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
    }
}

TEST(RequestParser_NonValidHeaders, InvalidInFirstRepeatedContentLength) {
	RequestParser	p;

	EXPECT_THROW(p.parseHttpRequest("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: 23s, 23\r\n\r\n"), std::runtime_error);
    p.clearParser();
    try{
        p.parseHttpRequest("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: 23s, 23\r\n\r\n");
    }
    catch(const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
    }
}

TEST(RequestParser_NonValidHeaders, InvalidInLastRepeatedContentLength) {
	RequestParser	p;

	EXPECT_THROW(p.parseHttpRequest("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: 23, e23\r\n\r\n"), std::runtime_error);
    p.clearParser();
    try{
        p.parseHttpRequest("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: 23, e23\r\n\r\n");
    }
    catch(const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
    }
}

TEST(RequestParser_NonValidHeaders, RepeatedContentLengthHeaders) {
	RequestParser	p;

	EXPECT_THROW(p.parseHttpRequest("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: 23\r\nContent-Length: 2\r\n\r\n"), std::runtime_error);
    p.clearParser();
    try{
        p.parseHttpRequest("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: 23\r\nContent-Length: 2\r\n\r\n");
    }
    catch(const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
    }
}

TEST(RequestParser_NonValidHeaders, EmptyTransferEncoding) {
	RequestParser	p;

	EXPECT_THROW(p.parseHttpRequest("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nTransfer-Encoding: \r\n\r\n"), std::runtime_error);
    p.clearParser();
    try{
        p.parseHttpRequest("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nTransfer-Encoding: \r\n\r\n");
    }
    catch(const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
    }
}

TEST(RequestParser_NonValidHeaders, InvalidChunkedTransferEncodingPositioning) {
	RequestParser	p;

	EXPECT_THROW(p.parseHttpRequest("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nTransfer-Encoding: chunked, gzip\r\n\r\n"), std::runtime_error);
    p.clearParser();
    try{
        p.parseHttpRequest("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nTransfer-Encoding: chunked, gzip\r\n\r\n");
    }
    catch(const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
    }
}

// PREVIOUS TEST CODE

// void	runHeaderTests(const std::string& name
// 		, size_t total
// 		, const std::pair<std::string, std::string> tests[])
// {
// 	std::cout << "\n* " << name << " *\n";
// 	for (size_t i = 0; i < total; i++) {
// 		std::cout << "\nTest [" << i << "]:\n" << tests[i].first << std::endl;
// 		try {
// 			RequestParser	p;
// 			HTTPRequest		request;
// 			std::string		key;
// 			std::string		value;

// 			request = p.parseHttpRequest(tests[i].first);
// 			std::istringstream	resultStream(tests[i].second);
			
// 			// Check header key values
// 			while (std::getline(resultStream, key)) {
// 				std::getline(resultStream, value);
// 				if (request.headers[key] == value)
// 					std::cout << "SUCCESS - " << key << ": " << request.headers[key] << "\n";
// 				else {
// 					std::cerr << "FAILURE - expected: " << value << "\n";
// 					std::cerr << "received: " << request.headers[key] << "\n";
// 				}
// 			}
// 		}
// 		catch (std::exception& e) {
// 			std::cerr << "Error: " << e.what() << std::endl;
// 		}
// 	} 
// }

// void	testValidHeader()
// {
// 	std::pair<std::string, std::string>	tests[] = {
// 		std::make_pair("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nUser-Agent: curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3\r\nAccept-Language: en, mi\r\n\r\n"
// 			, "Host\nwww.example.com\nUser-Agent\ncurl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3\nAccept-Language\nen, mi"),
// 		std::make_pair("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost:       www.example.com       \r\n\r\n"
// 			, "Host\nwww.example.com"),
// 		std::make_pair("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: 23\r\n\r\n"
// 			, "Host\nwww.example.com\nContent-Length\n23"),
// 		std::make_pair("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: 23, 23\r\n\r\n"
// 			, "Host\nwww.example.com\nContent-Length\n23"),
// 		std::make_pair("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nTransfer-Encoding: gzip, chunked\r\n\r\n0\r\n\r\n"
// 			, "Host\nwww.example.com\nTransfer-Encoding\ngzip, chunked"),
// 		std::make_pair("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nTransfer-Encoding: gzip\r\n\r\n"
// 			, "Host\nwww.example.com\nTransfer-Encoding\ngzip"),
// 	};
// 	runHeaderTests("VALID HEADER", sizeof(tests) / sizeof(tests[0]), tests);
// }

// void	testInvalidHeader()
// {
// 	std::pair<std::string, std::string>	tests[] = {
// 		std::make_pair("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost :       www.example.com       \r\n\r\n"
// 			, "Host\nwww.example.com"),
// 		std::make_pair("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\n Host:       www.example.com       \r\n\r\n"
// 			, "Host\nwww.example.com"),
// 		std::make_pair("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nH<ost: www.example.com\r\n\r\n"
// 			, "Host\nwww.example.com"),
// 		std::make_pair("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\n"
// 			, "Host\nwww.example.com"),
// 		std::make_pair("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: \r\n\r\n"
// 			, "Host\nwww.example.com\nContent-Length\n0"),
// 		std::make_pair("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: ur\r\n\r\n"
// 			, "Host\nwww.example.com\nContent-Length\n0"),
// 		std::make_pair("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: 23, 1\r\n\r\n"
// 			, "Host\nwww.example.com\nContent-Length\n23"),
// 		std::make_pair("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: 23s, 23\r\n\r\n"
// 			, "Host\nwww.example.com\nContent-Length\n23"),
// 		std::make_pair("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: 23, e23\r\n\r\n"
// 			, "Host\nwww.example.com\nContent-Length\n23"),
// 		std::make_pair("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: 23\r\nContent-Length: 2\r\n\r\n"
// 			, "Host\nwww.example.com\nContent-Length\n23"),
// 		std::make_pair("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nTransfer-Encoding: \r\n\r\n"
// 			, "Host\nwww.example.com\nTransfer-Encoding\n"),
// 		std::make_pair("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nTransfer-Encoding: chunked, gzip\r\n\r\n"
// 			, "Host\nwww.example.com\nTransfer-Encoding\nchunked, gzip"),
// 	};
// 	runHeaderTests("INVALID HEADER", sizeof(tests) / sizeof(tests[0]), tests);
// }
