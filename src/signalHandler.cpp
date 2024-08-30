#include "signalHandler.hpp"

volatile std::sig_atomic_t g_SignalStatus = 0;

void signalHandler(int signal)
{
	g_SignalStatus = signal;
}
