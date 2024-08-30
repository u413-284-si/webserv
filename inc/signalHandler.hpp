#pragma once

#include <csignal>

extern volatile std::sig_atomic_t g_SignalStatus;

extern "C" void signalHandler(int signal);
