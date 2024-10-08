#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "EpollWrapper.hpp"

/**
 * @brief Mock class for EpollWrapper.
 *
 */
class MockEpollWrapper : public EpollWrapper {
public:
	MOCK_METHOD(int, waitForEvents, (), (override));
	MOCK_METHOD(std::vector<struct epoll_event>::const_iterator, eventsBegin, (), (const, override));
	MOCK_METHOD(bool, addEvent, (int, uint32_t), (const, override));
	MOCK_METHOD(bool, modifyEvent, (int, uint32_t), (const, override));
	MOCK_METHOD(void, removeEvent, (int), (const, override));
};
