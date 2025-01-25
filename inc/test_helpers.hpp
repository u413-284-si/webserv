#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "MockEpollWrapper.hpp"
#include "MockFileSystemOps.hpp"
#include "MockProcessOps.hpp"
#include "MockSocketOps.hpp"
#include "Server.hpp"

using ::testing::NiceMock;

ConfigFile createTestConfigfile();

class ServerTestBase : public ::testing::Test {
protected:
	ServerTestBase() { }
	~ServerTestBase() override { }

	ConfigFile m_configFile = createTestConfigfile();
	NiceMock<MockEpollWrapper> m_epollWrapper;
	MockFileSystemOps m_fileSystemOps;
	MockSocketOps m_socketOps;
	MockProcessOps m_processOps;
	Server m_server = Server(m_configFile, m_epollWrapper, m_fileSystemOps, m_socketOps, m_processOps);
};
