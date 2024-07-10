#include "FileHandler.hpp"

FileHandler::~FileHandler() {}

FileHandler::fileType FileHandler::checkFileType(const std::string& path) const
{
	struct stat fileStat = {};
	errno = 0;
	if (stat(path.c_str(), &fileStat) == -1) {
		if (errno == ENOENT)
			return FileNotExist;
		std::cerr << "error: stat: " << strerror(errno) << "\n";
		return StatError;
	}
	if (S_ISREG(fileStat.st_mode))
		return FileRegular;
	if (S_ISDIR(fileStat.st_mode))
		return FileDirectory;
	return FileOther;
}

bool FileHandler::isDirectory(const std::string& path) const { return checkFileType(path) == FileDirectory; }

bool FileHandler::isExistingFile(const std::string& path) const { return checkFileType(path) != FileNotExist; }

std::string FileHandler::getFileContents(const char* filename) const
{
	std::ifstream fileStream(filename, std::ios::in | std::ios::binary);
	if (!fileStream.good()) {
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
