#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "FileSystemOps.hpp"

/**
 * @brief Mock class for FileSystemOps.
 *
 */
class MockFileSystemOps : public FileSystemOps {
public:
	MOCK_METHOD(bool, isDirectory, (const std::string&), (const, override));
	MOCK_METHOD(bool, isExistingFile, (const std::string&), (const, override));
	MOCK_METHOD(std::string, getFileContents, (const char*), (const, override));
	MOCK_METHOD(fileType, checkFileType, (const std::string&), (const, override));
	MOCK_METHOD(DIR*, openDirectory, (const std::string&), (const, override));
	MOCK_METHOD(struct dirent*, readDirectory, (DIR*), (const, override));
	MOCK_METHOD(int, closeDirectory, (DIR*), (const, override));
	MOCK_METHOD(struct stat, getFileStat, (const std::string&), (const, override));
	MOCK_METHOD(void, writeToFile, (const std::string&, const std::string&), (const, override));
};
