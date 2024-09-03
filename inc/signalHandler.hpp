#pragma once

#include <csignal>
#include <string>

// NOLINTNEXTLINE: This is a global variable used in signalHandler
extern volatile std::sig_atomic_t g_signalStatus;

extern "C" void signalHandler(int signal);

std::string signalNumToName(int signum);
