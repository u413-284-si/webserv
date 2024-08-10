#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdexcept>

#include "MockFileSystemPolicy.hpp"

#include "ConfigFile.hpp"
#include "HTTPResponse.hpp"
#include "RequestParser.hpp"
#include "StatusCode.hpp"
#include "TargetResourceHandler.hpp"

class TargetResourceHandlerTest : public ::testing::Test {
protected:
	TargetResourceHandlerTest()
	{
		Location m_location1;
		Location m_location2;
		Location m_location3;
		Location m_location4;

		m_location1.setPath("/");
		m_location1.setRoot("/first/location");
		m_location2.setPath("/test");
		m_location2.setRoot("/second/location");
		m_location3.setPath("/test/secret");
		m_location3.setRoot("/third/location");
		m_location3.setIndex("index.html");
		m_location3.setIndex("index.htm");
		m_location4.setPath("/test/autoindex");
		m_location4.setRoot("/fourth/location");
		m_location4.setIsAutoindex(true);

		m_locations.push_back(m_location3);
		m_locations.push_back(m_location2);
		m_locations.push_back(m_location4);
		m_locations.push_back(m_location1);
	}
	~TargetResourceHandlerTest() override { }

	std::vector<Location> m_locations;
	HTTPRequest m_request = { .method = MethodGet, .uri = { .path = "/test" } };
	HTTPResponse m_response = { .status = StatusOK, .isAutoindex = false };
	MockFileSystemPolicy m_fileSystemPolicy;
};

TEST_F(TargetResourceHandlerTest, FindCorrectLocation)
{
	EXPECT_CALL(m_fileSystemPolicy, checkFileType).WillRepeatedly(testing::Return(FileSystemPolicy::FileRegular));

	TargetResourceHandler targetResourceHandler(m_locations, m_request, m_response, m_fileSystemPolicy);
	targetResourceHandler.execute();
	EXPECT_EQ(m_response.targetResource, "/second/location/test");
	EXPECT_EQ(m_response.status, StatusOK);

	m_request.uri.path = "/test/secret";
	m_response = { .status = StatusOK };
	targetResourceHandler.execute();
	EXPECT_EQ(m_response.targetResource, "/third/location/test/secret");
	EXPECT_EQ(m_response.status, StatusOK);

	m_request.uri.path = "/test/secret/other";
	m_response = { .status = StatusOK };
	targetResourceHandler.execute();
	EXPECT_EQ(m_response.targetResource, "/third/location/test/secret/other");
	EXPECT_EQ(m_response.status, StatusOK);

	m_request.uri.path = "/";
	m_response = { .status = StatusOK };
	targetResourceHandler.execute();
	EXPECT_EQ(m_response.targetResource, "/first/location/");
	EXPECT_EQ(m_response.status, StatusOK);
}

TEST_F(TargetResourceHandlerTest, LocationNotFound)
{
	m_locations.pop_back();

	m_request.uri.path = "/something";

	TargetResourceHandler targetResourceHandler(m_locations, m_request, m_response, m_fileSystemPolicy);
	targetResourceHandler.execute();

	EXPECT_EQ(m_response.status, StatusNotFound);
	EXPECT_EQ(m_response.targetResource, "");
}

TEST_F(TargetResourceHandlerTest, FileNotFound)
{
	EXPECT_CALL(m_fileSystemPolicy, checkFileType).WillOnce(testing::Return(FileSystemPolicy::FileNotExist));

	m_request.uri.path = "/test";

	TargetResourceHandler targetResourceHandler(m_locations, m_request, m_response, m_fileSystemPolicy);
	targetResourceHandler.execute();

	EXPECT_EQ(m_response.status, StatusNotFound);
	EXPECT_EQ(m_response.targetResource, "/second/location/test");
}

TEST_F(TargetResourceHandlerTest, DirectoryRedirect)
{
	EXPECT_CALL(m_fileSystemPolicy, checkFileType).WillOnce(testing::Return(FileSystemPolicy::FileDirectory));

	m_request.uri.path = "/test";

	TargetResourceHandler targetResourceHandler(m_locations, m_request, m_response, m_fileSystemPolicy);
	targetResourceHandler.execute();

	EXPECT_EQ(m_response.status, StatusMovedPermanently);
	EXPECT_EQ(m_response.targetResource, "/second/location/test/");
}

TEST_F(TargetResourceHandlerTest, DirectoryIndex)
{
	EXPECT_CALL(m_fileSystemPolicy, checkFileType)
		.WillOnce(testing::Return(FileSystemPolicy::FileDirectory))
		.WillOnce(testing::Return(FileSystemPolicy::FileRegular));

	m_request.uri.path = "/test/secret/";

	TargetResourceHandler targetResourceHandler(m_locations, m_request, m_response, m_fileSystemPolicy);
	targetResourceHandler.execute();

	EXPECT_EQ(m_response.status, StatusOK);
	EXPECT_EQ(m_response.targetResource, "/third/location/test/secret/index.html");
}

TEST_F(TargetResourceHandlerTest, DirectoryAutoIndex)
{
	EXPECT_CALL(m_fileSystemPolicy, checkFileType).WillOnce(testing::Return(FileSystemPolicy::FileDirectory));

	m_request.uri.path = "/test/autoindex/";

	TargetResourceHandler targetResourceHandler(m_locations, m_request, m_response, m_fileSystemPolicy);
	targetResourceHandler.execute();

	EXPECT_EQ(m_response.status, StatusOK);
	EXPECT_EQ(m_response.targetResource, "/fourth/location/test/autoindex/");
	EXPECT_TRUE(m_response.isAutoindex);
}

TEST_F(TargetResourceHandlerTest, DirectoryForbidden)
{
	EXPECT_CALL(m_fileSystemPolicy, checkFileType).WillOnce(testing::Return(FileSystemPolicy::FileDirectory));

	m_request.uri.path = "/test/";

	TargetResourceHandler targetResourceHandler(m_locations, m_request, m_response, m_fileSystemPolicy);
	targetResourceHandler.execute();

	EXPECT_EQ(m_response.status, StatusForbidden);
	EXPECT_EQ(m_response.targetResource, "/second/location/test/");
	EXPECT_FALSE(m_response.isAutoindex);
}

TEST_F(TargetResourceHandlerTest, ServerError)
{
	EXPECT_CALL(m_fileSystemPolicy, checkFileType).WillOnce(testing::Throw(std::runtime_error("Stat error")));

	m_request.uri.path = "/test/";

	TargetResourceHandler targetResourceHandler(m_locations, m_request, m_response, m_fileSystemPolicy);
	targetResourceHandler.execute();

	EXPECT_EQ(m_response.status, StatusInternalServerError);
	EXPECT_EQ(m_response.targetResource, "/second/location/test/");
	EXPECT_FALSE(m_response.isAutoindex);
}
