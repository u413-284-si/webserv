#include "utils.hpp"

namespace utils {

std::string getFileExtension(const std::string& path)
{
	const std::size_t extPos = path.find_last_of('.');
	std::size_t dirPos = path.find_last_of('/');
	if (dirPos == std::string::npos) {
		dirPos = 0;
	}

	if (extPos != std::string::npos && extPos > dirPos) {
		return path.substr(extPos + 1);
	}
	return "";
}

} // namespace utils
