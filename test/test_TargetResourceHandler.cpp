#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "ConfigFile.hpp"
#include "FileHandler.hpp"
#include "HTTPResponse.hpp"
#include "RequestParser.hpp"
#include "TargetResourceHandler.hpp"

class FileHandlerMock : public FileHandler {
public:
	MOCK_METHOD(bool, isDirectory, (const std::string&), (const, override));
	MOCK_METHOD(bool, isExistingFile, (const std::string&), (const, override));
	MOCK_METHOD(std::string, getFileContents, (const char*), (const, override));
	MOCK_METHOD(fileType, checkFileType, (const std::string&), (const, override));
};

class TargetResourceHandlerTest : public ::testing::Test {
	protected:
	TargetResourceHandlerTest() { }
	~TargetResourceHandlerTest() override { }

	std::vector<Location> m_locations;
	Location m_location1 = {
		.path= "/",
		.root= "/first/location" };
	Location m_location2 = {
		.path= "/test",
		.root= "/second/location" };
	Location m_location3 = {
		.path= "/test/secret",
		.root= "/third/location",
		.index= "index.html" };
	HTTPRequest m_request = {
		.method = "GET",
		.uri = {
			.path = "/test"
		}
	};
	FileHandlerMock m_fileHandler;

};

TEST_F(TargetResourceHandlerTest, FindCorrectLocation)
{
	EXPECT_CALL(m_fileHandler, checkFileType)
	.WillRepeatedly(testing::Return(FileHandler::FileRegular));

	m_locations.push_back(m_location3);
	m_locations.push_back(m_location2);
	m_locations.push_back(m_location1);

	TargetResourceHandler targetResourceHandler(m_locations, m_fileHandler);

	EXPECT_EQ(targetResourceHandler.execute(m_request).targetResource, "/second/location/test");

	m_request.uri.path = "/test/secret";

	EXPECT_EQ(targetResourceHandler.execute(m_request).targetResource, "/third/location/test/secret");

	m_request.uri.path = "/test/secret/other";

	EXPECT_EQ(targetResourceHandler.execute(m_request).targetResource, "/third/location/test/secret/other");

	m_request.uri.path = "/";

	EXPECT_EQ(targetResourceHandler.execute(m_request).targetResource, "/first/location/");
}

TEST_F(TargetResourceHandlerTest, LocationNotFound)
{
	m_locations.push_back(m_location2);
	m_locations.push_back(m_location3);

	TargetResourceHandler targetResourceHandler(m_locations, m_fileHandler);

	m_request.uri.path = "/something";

	EXPECT_EQ(targetResourceHandler.execute(m_request).status, StatusNotFound);
}

TEST_F(TargetResourceHandlerTest, FileNotFound)
{
	EXPECT_CALL(m_fileHandler, checkFileType)
	.WillOnce(testing::Return(FileHandler::FileNotExist));

	m_locations.push_back(m_location1);
	m_locations.push_back(m_location2);
	m_locations.push_back(m_location3);

	TargetResourceHandler targetResourceHandler(m_locations, m_fileHandler);

	m_request.uri.path = "/test";

	EXPECT_EQ(targetResourceHandler.execute(m_request).status, StatusNotFound);
}

TEST_F(TargetResourceHandlerTest, DirectoryRedirect)
{
	EXPECT_CALL(m_fileHandler, checkFileType)
	.WillOnce(testing::Return(FileHandler::FileDirectory));

	m_locations.push_back(m_location1);
	m_locations.push_back(m_location2);
	m_locations.push_back(m_location3);

	TargetResourceHandler targetResourceHandler(m_locations, m_fileHandler);

	m_request.uri.path = "/test";

	HTTPResponse response = targetResourceHandler.execute(m_request);

	EXPECT_EQ(response.status, StatusMovedPermanently);
	EXPECT_EQ(response.targetResource, "/second/location/test/");
}

TEST_F(TargetResourceHandlerTest, DirectoryIndex)
{
	EXPECT_CALL(m_fileHandler, checkFileType)
	.WillOnce(testing::Return(FileHandler::FileDirectory))
	.WillOnce(testing::Return(FileHandler::FileRegular));

	m_locations.push_back(m_location1);
	m_locations.push_back(m_location2);
	m_locations.push_back(m_location3);

	TargetResourceHandler targetResourceHandler(m_locations, m_fileHandler);

	m_request.uri.path = "/test/secret/";

	const HTTPResponse response = targetResourceHandler.execute(m_request);

	EXPECT_EQ(response.status, StatusOK);
	EXPECT_EQ(response.targetResource, "/third/location/test/secret/index.html");
}
