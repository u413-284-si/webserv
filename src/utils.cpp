#include "utils.hpp"

namespace utils {

fileType checkFileType(const std::string& path)
{
	struct stat fileStat = {};
	errno = 0;
	if (stat(path.c_str(), &fileStat) == -1) {
		std::cerr << "error: stat: " << strerror(errno) << "\n";
		return NotExist;
	}
	if (S_ISREG(fileStat.st_mode))
		return RegularFile;
	if (S_ISDIR(fileStat.st_mode))
		return Directory;
	return Other;
}

bool isDirectory(const std::string& path) { return checkFileType(path) == Directory; }

bool isExistingFile(const std::string& path) { return checkFileType(path) != NotExist; }

std::string getFileContents(const char* filename)
{
	std::ifstream fileStream(filename, std::ios::in | std::ios::binary);
	if (fileStream != 0) {
		std::string contents;
		fileStream.seekg(0, std::ios::end);
		contents.resize(fileStream.tellg());
		fileStream.seekg(0, std::ios::beg);
		fileStream.read(&contents[0], static_cast<std::streamsize>(contents.size()));
		fileStream.close();
		return contents;
	}
	throw std::runtime_error(strerror(errno));
}

} // namespace utils
