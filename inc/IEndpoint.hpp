#pragma once

#include <ctime>

class Dispatcher;

class IEndpoint {
public:
	virtual ~IEndpoint() { }
	virtual void handleEvent(Dispatcher& dispatcher) = 0;
	virtual time_t getTimeSinceLastEvent() const = 0;
};
