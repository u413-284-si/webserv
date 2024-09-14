#include "signalHandler.hpp"
#include "Log.hpp"

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

/**
 * @brief Registers signals to the signal handler.
 *
 * Registered signals are:
 * - SIGINT: Quick shutdown
 * - SIGTERM: Quick shutdown
 * - SIGHUP: Quick shutdown
 * - SIGQUIT: Graceful shutdown
 * Where a quick shutdown is a shutdown that doesn't wait for the current connections to finish.
 * A graceful shutdown is a shutdown that waits for the current connections to finish.
 *
 * Regarding NOLINT: Return of SIG_ERR in case of failure of std::signal() is specified in man page.
 * The registering of a signal generates two errors: C-Style cast and downcast impacts.
 * 1. Reinterpret_cast would generate another error, so just deactivated.
 * 2. There is no performance lost by downcasting, since it's a simple check.
 *
 * @return true If all signals were registered successfully.
 * @return false If any signal failed to register.
 */
bool registerSignals()
{
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast, performance-no-int-to-ptr)
	if (std::signal(SIGINT, signalHandler) == SIG_ERR) {
		LOG_ERROR << "failed to register " << signalNumToName(SIGINT) << ": " << std::strerror(errno);
		return (false);
	}

	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast, performance-no-int-to-ptr)
	if (std::signal(SIGTERM, signalHandler) == SIG_ERR) {
		LOG_ERROR << "failed to register " << signalNumToName(SIGTERM) << ": " << std::strerror(errno);
		return (false);
	}

	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast, performance-no-int-to-ptr)
	if (std::signal(SIGHUP, signalHandler) == SIG_ERR) {
		LOG_ERROR << "failed to register " << signalNumToName(SIGHUP) << ": " << std::strerror(errno);
		return (false);
	}

	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast, performance-no-int-to-ptr)
	if (std::signal(SIGQUIT, signalHandler) == SIG_ERR) {
		LOG_ERROR << "failed to register " << signalNumToName(SIGQUIT) << ": " << std::strerror(errno);
		return (false);
	}

	return (true);
}
