#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "Server.hpp"
#include "MockEpollWrapper.hpp"
#include "MockFileSystemPolicy.hpp"
#include "MockSocketPolicy.hpp"
#include "MockProcessOps.hpp"

using ::testing::NiceMock;

ConfigFile createTestConfigfile();

class ServerTestBase : public ::testing::Test {
protected:
	ServerTestBase() {}
	~ServerTestBase() override {}

	ConfigFile m_configFile = createTestConfigfile();
	NiceMock<MockEpollWrapper> m_epollWrapper;
	MockFileSystemPolicy m_fileSystemPolicy;
	MockSocketPolicy m_socketPolicy;
	MockProcessOps m_processOps;
	Server m_server = Server(m_configFile, m_epollWrapper, m_fileSystemPolicy, m_socketPolicy, m_processOps);
};
