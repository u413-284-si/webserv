#pragma once

#include "Dispatcher.hpp"

class IEndpoint {
	public:
		virtual ~IEndpoint() {}
		virtual void handleEvent(Dispatcher& dispatcher) = 0;
};
