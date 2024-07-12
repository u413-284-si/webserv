#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "MockFileSystemPolicy.hpp"

#include "ConfigFile.hpp"
#include "HTTPResponse.hpp"
#include "RequestParser.hpp"
#include "TargetResourceHandler.hpp"

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
	MockFileSystemPolicy m_fileSystemPolicy;

};

TEST_F(TargetResourceHandlerTest, FindCorrectLocation)
{
	EXPECT_CALL(m_fileSystemPolicy, checkFileType)
	.WillRepeatedly(testing::Return(FileSystemPolicy::FileRegular));

	m_locations.push_back(m_location3);
	m_locations.push_back(m_location2);
	m_locations.push_back(m_location1);

	TargetResourceHandler targetResourceHandler(m_locations, m_fileSystemPolicy);

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

	TargetResourceHandler targetResourceHandler(m_locations, m_fileSystemPolicy);

	m_request.uri.path = "/something";

	EXPECT_EQ(targetResourceHandler.execute(m_request).status, StatusNotFound);
}

TEST_F(TargetResourceHandlerTest, FileNotFound)
{
	EXPECT_CALL(m_fileSystemPolicy, checkFileType)
	.WillOnce(testing::Return(FileSystemPolicy::FileNotExist));

	m_locations.push_back(m_location1);
	m_locations.push_back(m_location2);
	m_locations.push_back(m_location3);

	TargetResourceHandler targetResourceHandler(m_locations, m_fileSystemPolicy);

	m_request.uri.path = "/test";

	EXPECT_EQ(targetResourceHandler.execute(m_request).status, StatusNotFound);
}

TEST_F(TargetResourceHandlerTest, DirectoryRedirect)
{
	EXPECT_CALL(m_fileSystemPolicy, checkFileType)
	.WillOnce(testing::Return(FileSystemPolicy::FileDirectory));

	m_locations.push_back(m_location1);
	m_locations.push_back(m_location2);
	m_locations.push_back(m_location3);

	TargetResourceHandler targetResourceHandler(m_locations, m_fileSystemPolicy);

	m_request.uri.path = "/test";

	HTTPResponse response = targetResourceHandler.execute(m_request);

	EXPECT_EQ(response.status, StatusMovedPermanently);
	EXPECT_EQ(response.targetResource, "/second/location/test/");
}

TEST_F(TargetResourceHandlerTest, DirectoryIndex)
{
	EXPECT_CALL(m_fileSystemPolicy, checkFileType)
	.WillOnce(testing::Return(FileSystemPolicy::FileDirectory))
	.WillOnce(testing::Return(FileSystemPolicy::FileRegular));

	m_locations.push_back(m_location1);
	m_locations.push_back(m_location2);
	m_locations.push_back(m_location3);

	TargetResourceHandler targetResourceHandler(m_locations, m_fileSystemPolicy);

	m_request.uri.path = "/test/secret/";

	const HTTPResponse response = targetResourceHandler.execute(m_request);

	EXPECT_EQ(response.status, StatusOK);
	EXPECT_EQ(response.targetResource, "/third/location/test/secret/index.html");
}
