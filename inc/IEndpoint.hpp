#pragma once

#include <ctime>
#include <stdint.h>

class Dispatcher;

class IEndpoint {
public:
	virtual ~IEndpoint() { }
	virtual void handleEvent(Dispatcher& dispatcher, uint32_t eventMask) = 0;
	virtual time_t getTimeSinceLastEvent() const = 0;
};
