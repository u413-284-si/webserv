#pragma once

#include <csignal>
#include <string>

extern volatile std::sig_atomic_t g_signalStatus;

extern "C" void signalHandler(int signal);

std::string signalNumToName(int signum);
