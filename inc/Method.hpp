#pragma once

#include "iostream"

enum Method { MethodGet, MethodPost, MethodDelete, MethodCount };

std::ostream& operator<<(std::ostream& ostream, Method method);
