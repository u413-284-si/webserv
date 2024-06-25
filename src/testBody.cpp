#include "test.hpp"

void	runBodyTests(const std::string& name
		, size_t total
		, const std::pair<std::string, std::string> tests[])
{
	std::cout << "\n* " << name << " *\n";
	for (size_t i = 0; i < total; i++) {
		std::cout << "\nTest [" << i << "]:\n" << tests[i].first << std::endl;
		try {
			RequestParser	p;
			HTTPRequest		request;
			std::string		body;

			request = p.parseHttpRequest(tests[i].first);
			
			// Check body string
			if (request.body == tests[i].second)
				std::cout << "SUCCESS - Body: " << request.body << "\n";
			else {
				std::cerr << "FAILURE - expected: " << tests[i].second << "\n";
				std::cerr << "received: " << request.body << "\n";
		}
		}
		catch (std::exception& e) {
			std::cerr << "Error: " << e.what() << std::endl;
		}
	} 
}

void	testValidBody()
{
	std::pair<std::string, std::string>	tests[] = {
		std::make_pair("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nTransfer-Encoding: gzip, chunked\r\n\r\n6\r\nhello \r\n6\r\nworld!\r\n0\r\n\r\n"
			, "hello world!"),

	};
	runBodyTests("VALID BODY", sizeof(tests) / sizeof(tests[0]), tests);
}

void	testInvalidBody()
{
	std::pair<std::string, std::string>	tests[] = {
		std::make_pair("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost :       www.example.com       \r\n\r\n"
			, "Host\nwww.example.com"),
	};
	runBodyTests("INVALID BODY", sizeof(tests) / sizeof(tests[0]), tests);
}
