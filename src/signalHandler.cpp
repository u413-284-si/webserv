#include "signalHandler.hpp"

// NOLINTNEXTLINE: This is a global variable used in signalHandler
volatile std::sig_atomic_t g_signalStatus = 0;

/**
 * @brief Simple signal handler that sets the global variable g_signalStatus to the signal number.
 *
 * @param signal The signal number.
 */
void signalHandler(int signal) { g_signalStatus = signal; }

/**
 * @brief Converts a signal number to its name.
 *
 * @param signum The signal number.
 * @return std::string The signal name.
 */
std::string signalNumToName(int signum)
{
	switch (signum) {
	case SIGHUP:
		return "SIGHUP";
	case SIGINT:
		return "SIGINT";
	case SIGQUIT:
		return "SIGQUIT";
	case SIGTERM:
		return "SIGTERM";
	default:
		return "Unknown signal " + webutils::toString(signum);
	}
}
