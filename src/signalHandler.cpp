#include "signalHandler.hpp"

//NOLINTNEXTLINE: This is a global variable used in signalHandler
volatile std::sig_atomic_t g_SignalStatus = 0;

void signalHandler(int signal)
{
	g_SignalStatus = signal;
}
