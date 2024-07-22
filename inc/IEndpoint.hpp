#pragma once

class Dispatcher;

class IEndpoint {
	public:
		virtual ~IEndpoint() {}
		virtual void handleEvent(Dispatcher& dispatcher) = 0;
};
