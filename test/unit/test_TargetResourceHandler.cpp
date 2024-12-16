#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ConfigFile.hpp"
#include "Connection.hpp"
#include "MockFileSystemPolicy.hpp"

#include "TargetResourceHandler.hpp"

class TargetResourceHandlerTest : public ::testing::Test {
protected:
	TargetResourceHandlerTest()
		: m_connection(Connection(m_serverSock, Socket(), dummyFd, m_configFile.servers))
		, m_targetResourceHandler(m_fileSystemPolicy)
	{
		Location m_location1;
		Location m_location2;
		Location m_location3;
		Location m_location4;
		Location m_location5;

		m_location1.path = "/";
		m_location1.root = "/first/location";
		m_location2.path = "/test";
		m_location2.root = "/second/location";
		m_location3.path = "/test/secret";
		m_location3.root = "/third/location";
		m_location3.indices.emplace_back("index.html");
		m_location3.indices.emplace_back("other.html");
		m_location4.path = "/test/autoindex";
		m_location4.root = "/fourth/location";
		m_location4.isAutoindex = true;
		m_location5.path = "/recursion";
		m_location5.root = "/start";
		m_location5.indices.emplace_back("again/");

		m_configFile.servers[0].locations.pop_back();
		m_configFile.servers[0].locations.push_back(m_location3);
		m_configFile.servers[0].locations.push_back(m_location2);
		m_configFile.servers[0].locations.push_back(m_location4);
		m_configFile.servers[0].locations.push_back(m_location1);
		m_configFile.servers[0].locations.push_back(m_location5);

		m_request.method = MethodGet;
		m_request.uri.path = "/test";
	}
	~TargetResourceHandlerTest() override { }

    const int dummyFd = 10;
	ConfigFile m_configFile = createDummyConfig();
	MockFileSystemPolicy m_fileSystemPolicy;
	Socket m_serverSock = { .host = "127.0.0.1", .port = "8080" };
	Connection m_connection;
	HTTPRequest& m_request = m_connection.m_request;
	TargetResourceHandler m_targetResourceHandler;
};

TEST_F(TargetResourceHandlerTest, FindCorrectLocation)
{
	EXPECT_CALL(m_fileSystemPolicy, checkFileType).WillRepeatedly(testing::Return(FileSystemPolicy::FileRegular));

	m_targetResourceHandler.execute(m_connection);

	EXPECT_EQ(m_request.targetResource, "/second/location/test");
	EXPECT_EQ(m_request.httpStatus, StatusOK);

	/*
		m_request.uri.path = "/test/secret";
		m_request.httpStatus = StatusOK;
		m_targetResourceHandler.execute(m_connection);
		EXPECT_EQ(m_request.targetResource, "/third/location/test/secret");
		EXPECT_EQ(m_request.httpStatus, StatusOK);

		m_request.uri.path = "/test/secret/other";
		m_request.httpStatus = StatusOK;
		m_targetResourceHandler.execute(m_connection);
		EXPECT_EQ(m_request.targetResource, "/third/location/test/secret/other");
		EXPECT_EQ(m_request.httpStatus, StatusOK);

		m_request.uri.path = "/";
		m_request.httpStatus = StatusOK;
		m_targetResourceHandler.execute(m_connection);
		EXPECT_EQ(m_request.targetResource, "/first/location/");
		EXPECT_EQ(m_request.httpStatus, StatusOK);
		*/
}

TEST_F(TargetResourceHandlerTest, FileNotFound)
{
	EXPECT_CALL(m_fileSystemPolicy, checkFileType)
	.WillOnce(testing::Throw(FileSystemPolicy::FileNotFoundException("File not found")));

	m_request.uri.path = "/test";

	m_targetResourceHandler.execute(m_connection);

	EXPECT_EQ(m_request.httpStatus, StatusNotFound);
	EXPECT_EQ(m_request.targetResource, "/second/location/test");
}

TEST_F(TargetResourceHandlerTest, FileOther)
{
	EXPECT_CALL(m_fileSystemPolicy, checkFileType)
	.WillOnce(testing::Return(FileSystemPolicy::FileOther));

	m_request.uri.path = "/test";

	m_targetResourceHandler.execute(m_connection);

	EXPECT_EQ(m_request.httpStatus, StatusForbidden);
	EXPECT_EQ(m_request.targetResource, "/second/location/test");
}

TEST_F(TargetResourceHandlerTest, FileNoPermission)
{
	EXPECT_CALL(m_fileSystemPolicy, checkFileType)
	.WillOnce(testing::Throw(FileSystemPolicy::NoPermissionException("No permission")));

	m_request.uri.path = "/test";

	m_targetResourceHandler.execute(m_connection);

	EXPECT_EQ(m_request.httpStatus, StatusForbidden);
	EXPECT_EQ(m_request.targetResource, "/second/location/test");
}

TEST_F(TargetResourceHandlerTest, DirectoryRedirect)
{
	EXPECT_CALL(m_fileSystemPolicy, checkFileType)
	.WillOnce(testing::Return(FileSystemPolicy::FileDirectory));

	m_request.uri.path = "/test";

	m_targetResourceHandler.execute(m_connection);

	EXPECT_EQ(m_request.httpStatus, StatusMovedPermanently);
	EXPECT_EQ(m_request.targetResource, "/second/location/test/");
}

TEST_F(TargetResourceHandlerTest, DirectoryIndex)
{
	EXPECT_CALL(m_fileSystemPolicy, checkFileType)
	.WillOnce(testing::Return(FileSystemPolicy::FileDirectory))
	.WillOnce(testing::Return(FileSystemPolicy::FileRegular));

	m_request.uri.path = "/test/secret/";

	m_targetResourceHandler.execute(m_connection);

	EXPECT_EQ(m_request.httpStatus, StatusOK);
	EXPECT_EQ(m_request.targetResource, "/third/location/test/secret/index.html");
}

TEST_F(TargetResourceHandlerTest, DirectoryIndexNotFound)
{
	EXPECT_CALL(m_fileSystemPolicy, checkFileType)
	.WillOnce(testing::Return(FileSystemPolicy::FileDirectory))
	.WillOnce(testing::Throw(FileSystemPolicy::FileNotFoundException("File not found")))
	.WillOnce(testing::Throw(FileSystemPolicy::FileNotFoundException("File not found")));

	m_request.uri.path = "/test/secret/";

	m_targetResourceHandler.execute(m_connection);

	EXPECT_EQ(m_request.httpStatus, StatusForbidden);
	EXPECT_EQ(m_request.targetResource, "/third/location/test/secret/");
}

TEST_F(TargetResourceHandlerTest, DirectoryIndexErrorInRecursion)
{
	EXPECT_CALL(m_fileSystemPolicy, checkFileType)
	.WillOnce(testing::Return(FileSystemPolicy::FileDirectory))
	.WillOnce(testing::Return(FileSystemPolicy::FileDirectory))
	.WillOnce(testing::Throw(std::runtime_error("Stat error")));

	m_request.uri.path = "/recursion/";

	m_targetResourceHandler.execute(m_connection);

	EXPECT_EQ(m_request.httpStatus, StatusInternalServerError);
	EXPECT_EQ(m_request.targetResource, "/start/recursion/again/again/");
}

TEST_F(TargetResourceHandlerTest, DirectoryIndexMaxRecursion)
{
	EXPECT_CALL(m_fileSystemPolicy, checkFileType)
	.WillRepeatedly(testing::Return(FileSystemPolicy::FileDirectory));

	m_request.uri.path = "/recursion/";

	m_targetResourceHandler.execute(m_connection);

	EXPECT_EQ(m_request.httpStatus, StatusInternalServerError);
	EXPECT_EQ(m_request.targetResource, "/start/recursion/again/again/again/again/again/again/again/again/");
}

TEST_F(TargetResourceHandlerTest, DirectoryAutoIndex)
{
	EXPECT_CALL(m_fileSystemPolicy, checkFileType)
	.WillOnce(testing::Return(FileSystemPolicy::FileDirectory));

	m_request.uri.path = "/test/autoindex/";

	m_targetResourceHandler.execute(m_connection);

	EXPECT_EQ(m_request.httpStatus, StatusOK);
	EXPECT_EQ(m_request.targetResource, "/fourth/location/test/autoindex/");
	EXPECT_TRUE(m_request.hasAutoindex);
}

TEST_F(TargetResourceHandlerTest, DirectoryForbidden)
{
	EXPECT_CALL(m_fileSystemPolicy, checkFileType)
	.WillOnce(testing::Return(FileSystemPolicy::FileDirectory));

	m_request.uri.path = "/test/";

	m_targetResourceHandler.execute(m_connection);

	EXPECT_EQ(m_request.httpStatus, StatusForbidden);
	EXPECT_EQ(m_request.targetResource, "/second/location/test/");
	EXPECT_FALSE(m_request.hasAutoindex);
}

TEST_F(TargetResourceHandlerTest, ServerError)
{
	EXPECT_CALL(m_fileSystemPolicy, checkFileType)
	.WillOnce(testing::Throw(std::runtime_error("Stat error")));

	m_request.uri.path = "/test/";

	m_targetResourceHandler.execute(m_connection);

	EXPECT_EQ(m_request.httpStatus, StatusInternalServerError);
	EXPECT_EQ(m_request.targetResource, "/second/location/test/");
	EXPECT_FALSE(m_request.hasAutoindex);
}
