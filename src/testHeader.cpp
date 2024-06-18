#include "test.hpp"

void	runHeaderTests(const std::string& name
		, size_t total
		, const std::pair<std::string, std::string> tests[])
{
	std::cout << "\n* " << name << " *\n";
	for (size_t i = 0; i < total; i++) {
		std::cout << "\nTest [" << i << "]:\n" << tests[i].first << std::endl;
		try {
			RequestParser	p;
			HTTPRequest		request;
			std::string		key;
			std::string		value;

			request = p.parseHttpRequest(tests[i].first);
			std::istringstream	resultStream(tests[i].second);
			
			// Check header key values
			while (std::getline(resultStream, key)) {
				std::getline(resultStream, value);
				if (request.headers[key] == value)
					std::cout << "SUCCESS - " << key << ": " << request.headers[key] << "\n";
				else {
					std::cerr << "FAILURE - expected: " << value << "\n";
					std::cerr << "received: " << request.headers[key] << "\n";
				}
			}
		}
		catch (std::exception& e) {
			std::cerr << "Error: " << e.what() << std::endl;
		}
	} 
}

void	testValidHeader()
{
	std::pair<std::string, std::string>	tests[] = {
		std::make_pair("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost: www.example.com\r\nUser-Agent: curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3\r\nAccept-Language: en, mi\r\n"
			, "Host\nwww.example.com\nUser-Agent\ncurl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3\nAccept-Language\nen, mi"),
		std::make_pair("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost:       www.example.com       \r\n"
			, "Host\nwww.example.com"),
	};
	runHeaderTests("VALID HEADER", sizeof(tests) / sizeof(tests[0]), tests);
}

void	testInvalidHeader()
{
	std::pair<std::string, std::string>	tests[] = {
		std::make_pair("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\nHost :       www.example.com       \r\n"
			, "Host\nwww.example.com"),
	};
	runHeaderTests("INVALID HEADER", sizeof(tests) / sizeof(tests[0]), tests);
}
