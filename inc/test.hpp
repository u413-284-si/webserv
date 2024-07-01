#pragma once

/* ====== LIBRARIES ====== */

#include "RequestParser.hpp"
#include "Server.hpp"

/* ====== DEFINITIONS ====== */

// Request Line Tests
void	runRequestLineTests(const std::string& name
		, size_t total
		, const std::pair<std::string, std::string> tests[]);
void	testValidRequestLine();
void	testInvalidRequestLine();

// Header Tests
void	runHeaderTests(const std::string& name
		, size_t total
		, const std::pair<std::string, std::string> tests[]);
void	testValidHeader();
void	testInvalidHeader();

// Body Tests
void	runBodyTests(const std::string& name
		, size_t total
		, const std::pair<std::string, std::string> tests[]);
void	testValidBody();
void	testInvalidBody();
