#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ConfigFile.hpp"
#include "Connection.hpp"
#include "MockFileSystemOps.hpp"

#include "TargetResourceHandler.hpp"

ConfigFile createTestConfigfile();

using ::testing::Return;
using ::testing::Throw;

class TargetResourceHandlerTest : public ::testing::Test {
protected:
	TargetResourceHandlerTest()
	{
		Location location1;
		location1.path = "/";
		location1.root = "/first/location";

		Location location2;
		location2.path = "/test";
		location2.root = "/second/location";

		Location location3;
		location3.path = "/test/secret";
		location3.root = "/third/location";
		location3.indices.emplace_back("index.html");
		location3.indices.emplace_back("other.html");

		Location location4;
		location4.path = "/test/autoindex";
		location4.root = "/fourth/location";
		location4.hasAutoindex = true;

		Location location5;
		location5.path = "/recursion";
		location5.root = "/fifth/location";
		location5.indices.emplace_back("again/");

		Location location6;
		location6.path = "/redirect";
		location6.root = "/sixth/location";
		location6.returns = std::make_pair(StatusMovedPermanently, "/newlocation");

		Location location7;
		location7.path = "/alias";
		location7.root = "/sixth/location";
		location7.alias = "/new/path";

		m_configFile.servers[0].locations.pop_back();
		m_configFile.servers[0].locations.push_back(location3);
		m_configFile.servers[0].locations.push_back(location2);
		m_configFile.servers[0].locations.push_back(location4);
		m_configFile.servers[0].locations.push_back(location1);
		m_configFile.servers[0].locations.push_back(location5);
		m_configFile.servers[0].locations.push_back(location6);
		m_configFile.servers[0].locations.push_back(location7);
	}
	~TargetResourceHandlerTest() override { }

	const int m_dummyFd = 10;
	Socket m_serverSock = { .host = "127.0.0.1", .port = "8080" };
	ConfigFile m_configFile = createTestConfigfile();
	Connection m_connection = Connection(m_serverSock, Socket(), m_dummyFd, m_configFile.servers);

	HTTPRequest& m_request = m_connection.m_request;

	MockFileSystemOps m_fileSystemOps;
	TargetResourceHandler m_targetResourceHandler = TargetResourceHandler(m_fileSystemOps);
};

TEST_F(TargetResourceHandlerTest, FindCorrectLocation)
{
	EXPECT_CALL(m_fileSystemOps, checkFileType).WillOnce(Return(FileSystemOps::FileRegular));

	m_request.uri.path = "/test";

	m_targetResourceHandler.execute(m_connection.m_request, m_connection.location, m_connection.serverConfig);

	EXPECT_EQ(m_request.targetResource, "/second/location/test");
	EXPECT_EQ(m_request.httpStatus, StatusOK);
}

TEST_F(TargetResourceHandlerTest, TwoLocationsMatchOneIsLonger)
{
	EXPECT_CALL(m_fileSystemOps, checkFileType).WillOnce(Return(FileSystemOps::FileRegular));

	m_request.uri.path = "/test/secret";

	m_targetResourceHandler.execute(m_connection.m_request, m_connection.location, m_connection.serverConfig);

	EXPECT_EQ(m_request.targetResource, "/third/location/test/secret");
	EXPECT_EQ(m_request.httpStatus, StatusOK);
}

TEST_F(TargetResourceHandlerTest, FileNotFound)
{
	EXPECT_CALL(m_fileSystemOps, checkFileType).WillOnce(Return(FileSystemOps::FileNotFound));

	m_request.uri.path = "/test";

	m_targetResourceHandler.execute(m_connection.m_request, m_connection.location, m_connection.serverConfig);

	EXPECT_EQ(m_request.httpStatus, StatusNotFound);
	EXPECT_EQ(m_request.targetResource, "/second/location/test");
}

TEST_F(TargetResourceHandlerTest, FileOther)
{
	EXPECT_CALL(m_fileSystemOps, checkFileType).WillOnce(Return(FileSystemOps::FileOther));

	m_request.uri.path = "/test";

	m_targetResourceHandler.execute(m_connection.m_request, m_connection.location, m_connection.serverConfig);

	EXPECT_EQ(m_request.httpStatus, StatusForbidden);
	EXPECT_EQ(m_request.targetResource, "/second/location/test");
}

TEST_F(TargetResourceHandlerTest, FileNoPermission)
{
	EXPECT_CALL(m_fileSystemOps, checkFileType)
	.WillOnce(testing::Throw(FileSystemOps::NoPermissionException("No permission")));

	m_request.uri.path = "/test";

	m_targetResourceHandler.execute(m_connection.m_request, m_connection.location, m_connection.serverConfig);

	EXPECT_EQ(m_request.httpStatus, StatusForbidden);
	EXPECT_EQ(m_request.targetResource, "/second/location/test");
}

TEST_F(TargetResourceHandlerTest, DirectoryRedirect)
{
	EXPECT_CALL(m_fileSystemOps, checkFileType).WillOnce(Return(FileSystemOps::FileDirectory));

	m_request.uri.path = "/test";

	m_targetResourceHandler.execute(m_connection.m_request, m_connection.location, m_connection.serverConfig);

	EXPECT_EQ(m_request.httpStatus, StatusMovedPermanently);
	EXPECT_TRUE(m_request.isDirectory);
	EXPECT_EQ(m_request.targetResource, "/test/");
}

TEST_F(TargetResourceHandlerTest, HitDirectoryAppendIndexFile)
{
	EXPECT_CALL(m_fileSystemOps, checkFileType)
		.WillOnce(Return(FileSystemOps::FileDirectory))
		.WillOnce(Return(FileSystemOps::FileRegular));

	m_request.uri.path = "/test/secret/";

	m_targetResourceHandler.execute(m_connection.m_request, m_connection.location, m_connection.serverConfig);

	EXPECT_EQ(m_request.httpStatus, StatusOK);
	EXPECT_EQ(m_request.targetResource, "/third/location/test/secret/index.html");
}

TEST_F(TargetResourceHandlerTest, HitDirectoryIndexfileNotFound)
{
	EXPECT_CALL(m_fileSystemOps, checkFileType)
	.WillOnce(Return(FileSystemOps::FileDirectory))
	.WillOnce(Return(FileSystemOps::FileNotFound))
	.WillOnce(Return(FileSystemOps::FileNotFound));

	m_request.uri.path = "/test/secret/";

	m_targetResourceHandler.execute(m_connection.m_request, m_connection.location, m_connection.serverConfig);

	EXPECT_EQ(m_request.httpStatus, StatusForbidden);
	EXPECT_TRUE(m_request.isDirectory);
	EXPECT_EQ(m_request.targetResource, "/third/location/test/secret/");
}

TEST_F(TargetResourceHandlerTest, DirectoryIndexErrorInRecursion)
{
	EXPECT_CALL(m_fileSystemOps, checkFileType)
		.WillOnce(Return(FileSystemOps::FileDirectory))
		.WillOnce(Return(FileSystemOps::FileDirectory))
		.WillOnce(Throw(std::runtime_error("Stat error")));

	m_request.uri.path = "/recursion/";

	m_targetResourceHandler.execute(m_connection.m_request, m_connection.location, m_connection.serverConfig);

	EXPECT_EQ(m_request.httpStatus, StatusInternalServerError);
	EXPECT_EQ(m_request.targetResource, "/fifth/location/recursion/again/again/");
}

TEST_F(TargetResourceHandlerTest, DirectoryIndexMaxRecursion)
{
	EXPECT_CALL(m_fileSystemOps, checkFileType).WillRepeatedly(Return(FileSystemOps::FileDirectory));

	m_request.uri.path = "/recursion/";

	m_targetResourceHandler.execute(m_connection.m_request, m_connection.location, m_connection.serverConfig);

	EXPECT_EQ(m_request.httpStatus, StatusInternalServerError);
	EXPECT_EQ(m_request.targetResource, "/fifth/location/recursion/again/again/again/again/again/again/again/again/");
}

TEST_F(TargetResourceHandlerTest, DirectoryAutoIndex)
{
	EXPECT_CALL(m_fileSystemOps, checkFileType).WillOnce(Return(FileSystemOps::FileDirectory));

	m_request.uri.path = "/test/autoindex/";

	m_targetResourceHandler.execute(m_connection.m_request, m_connection.location, m_connection.serverConfig);

	EXPECT_EQ(m_request.httpStatus, StatusOK);
	EXPECT_EQ(m_request.targetResource, "/fourth/location/test/autoindex/");
	EXPECT_TRUE(m_request.hasAutoindex);
	EXPECT_TRUE(m_request.isDirectory);
}

TEST_F(TargetResourceHandlerTest, DirectoryWithNoAutoindex)
{
	EXPECT_CALL(m_fileSystemOps, checkFileType).WillOnce(Return(FileSystemOps::FileDirectory));

	m_request.uri.path = "/test/";

	m_targetResourceHandler.execute(m_connection.m_request, m_connection.location, m_connection.serverConfig);

	EXPECT_EQ(m_request.httpStatus, StatusForbidden);
	EXPECT_EQ(m_request.targetResource, "/second/location/test/");
	EXPECT_FALSE(m_request.hasAutoindex);
	EXPECT_TRUE(m_request.isDirectory);
}

TEST_F(TargetResourceHandlerTest, ServerError)
{
	EXPECT_CALL(m_fileSystemOps, checkFileType).WillOnce(Throw(std::runtime_error("Stat error")));

	m_request.uri.path = "/test/";

	m_targetResourceHandler.execute(m_connection.m_request, m_connection.location, m_connection.serverConfig);

	EXPECT_EQ(m_request.httpStatus, StatusInternalServerError);
	EXPECT_EQ(m_request.targetResource, "/second/location/test/");
	EXPECT_FALSE(m_request.hasAutoindex);
}

TEST_F(TargetResourceHandlerTest, SimpleRedirection)
{
	m_request.uri.path = "/redirect";

	m_targetResourceHandler.execute(m_connection.m_request, m_connection.location, m_connection.serverConfig);

	EXPECT_EQ(m_request.httpStatus, StatusMovedPermanently);
	EXPECT_EQ(m_request.targetResource, "/newlocation");
	EXPECT_TRUE(m_request.hasReturn);
}

TEST_F(TargetResourceHandlerTest, LocationWithAlias)
{
	EXPECT_CALL(m_fileSystemOps, checkFileType).WillOnce(testing::Return(FileSystemOps::FileRegular));

	m_request.uri.path = "/alias/test";

	m_targetResourceHandler.execute(m_connection.m_request, m_connection.location, m_connection.serverConfig);

	EXPECT_EQ(m_request.targetResource, "/new/path/test");
	EXPECT_EQ(m_request.httpStatus, StatusOK);
}
