#include "RequestParser.hpp"
#include "Server.hpp"


void	runTests(const std::string& name
		, size_t total
		, const std::pair<std::string, std::string> tests[])
{
	std::cout << "\n* " << name << " *\n";
	for (size_t i = 0; i < total; i++) {
		std::cout << "\nTest [" << i << "]:\n" << tests[i].first << std::endl;
		try {
			RequestParser	p;
			HTTPRequest		request;
			std::string		result;

			request = p.parseHttpRequest(tests[i].first);
			std::istringstream	resultStream(tests[i].second);
			
			// Check method
			std::getline(resultStream, result);
			if (request.method == result)
				std::cout << "SUCCESS - Method: " << request.method << "\n";
			else {
				std::cerr << "FAILURE - expected: " << result << "\n";
				std::cerr << "received: " << request.method << "\n";
			}

			// Check path
			std::getline(resultStream, result);
			if (request.uri.path == result)
				std::cout << "SUCCESS - Path: " << request.uri.path << "\n";
			else {
				std::cerr << "FAILURE - expected: " << result << "\n";
				std::cerr << "received: " << request.uri.path << "\n";
			}

			// Check query
			std::getline(resultStream, result);
			if (request.uri.query == result)
				std::cout << "SUCCESS - Query: " << request.uri.query << "\n";
			else {
				std::cerr << "FAILURE - expected: " << result << "\n";
				std::cerr << "received: " << request.uri.query << "\n";
			}

			// Check fragment
			std::getline(resultStream, result);
			if (request.uri.fragment == result)
				std::cout << "SUCCESS - Fragment: " << request.uri.fragment << "\n";
			else {
				std::cerr << "FAILURE - expected: " << result << "\n";
				std::cerr << "received: " << request.uri.fragment << "\n";
			}

			// Check version
			std::getline(resultStream, result);
			if (request.version == result)
				std::cout << "SUCCESS - Version: " << request.version << "\n";
			else {
				std::cerr << "FAILURE - expected: " << result << "\n";
				std::cerr << "received: " << request.version << "\n";
			}
		}
		catch (std::exception& e) {
			std::cerr << "Error: " << e.what() << std::endl;
		}
	} 
}

void	testValidRequestLine() {
	std::pair<std::string, std::string>	tests[] = {
		std::make_pair("GET /search?query=openai&year=2024#conclusion HTTP/1.1\r\n"
			, "GET\n/search\nquery=openai&year=2024\nconclusion\n1.1"),
		std::make_pair("DELETE /index.html HTTP/1.1\r\n"
			, "DELETE\n/index.html\n\n\n1.1"),
		std::make_pair("POST /abracadabra/ipsum?user=aziz&key=password HTTP/2.0\r\n"
			, "POST\n/abracadabra/ipsum\nuser=aziz&key=password\n\n2.0"),
	};
	runTests("VALID REQUEST LINES", sizeof(tests) / sizeof(tests[0]), tests);
}

int	main()
{
	testValidRequestLine();
	return 0;
}
