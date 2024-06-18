#include "test.hpp"

void	testValidHeader() {
	std::pair<std::string, std::string>	tests[] = {
		std::make_pair("PUT /search?query=openai&year=2024#conclusion HTTP/1.1\r\n"
			, "PUT\n/search\nquery=openai&year=2024\nconclusion\n1.1"),
	};
	runRequestLineTests("VALID REQUEST LINES", sizeof(tests) / sizeof(tests[0]), tests);
}
	
int	main()
{
	testValidRequestLine();
	testInvalidRequestLine();
	return 0;
}
