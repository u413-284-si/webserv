#include "utils.hpp"

namespace utils
{

fileType checkFileType(const std::string& path)
{
	struct stat fileStat = {};
	errno = 0;
	if (stat(path.c_str(), &fileStat) == -1)
	{
		std::cerr << "error: stat: " << strerror(errno) << "\n";
		return NotExist;
	}
	if (S_ISREG(fileStat.st_mode))
		return RegularFile;
	if (S_ISDIR(fileStat.st_mode))
		return Directory;
	return Other;
}

} // namespace utils
