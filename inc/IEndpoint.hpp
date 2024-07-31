#pragma once

#include <ctime>
#include <stdint.h>
#include <string>

class Dispatcher;

class IEndpoint {
public:
	virtual ~IEndpoint() { }
	virtual void handleEvent(Dispatcher& dispatcher, uint32_t eventMask) = 0;
	virtual time_t getTimeSinceLastEvent() const = 0;
	virtual bool isActive() const = 0;
	virtual std::string getType() const = 0;
};
