# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: gwolf <gwolf@student.42vienna.com>         +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/07/28 13:03:05 by gwolf             #+#    #+#              #
#    Updated: 2024/08/28 15:00:05 by gwolf            ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# ******************************
# *     Targets                *
# ******************************

NAME := webserv

UNIT := unittest

NAME_SANI := $(NAME)_sani

UNIT_SANI := $(UNIT)_sani

# ******************************
# *     Variables              *
# ******************************

# Set VERBOSE=1 to echo all commands
ifeq ($(VERBOSE),1)
	SILENT =
else
	SILENT = @
endif

# Set SANI=1 to enable sanitizers
ifeq ($(SANI),1)
	ASAN = -fsanitize=address,undefined
else
	ASAN =
endif

# Set NODEBUG=1 to compile without debug messages
ifeq ($(NODEBUG),1)
	DEBUG_FLAGS =
else
	DEBUG_FLAGS = -D DEBUG_MSG
endif

# ******************************
# *     Text effects           *
# ******************************

RESET := \033[0m
BOLD := \033[1m
BLACK := \033[30m
GREEN := \033[32m
YELLOW := \033[33m
RED := \033[31m
BLUE := \033[34m

# ******************************
# *     Directories            *
# ******************************

SRC_DIR := src

# Base directory for object files
BASE_OBJ_DIR = obj

# Changes object directory and target names when SANI=1
# This ensures, that the correct object files are linked object and
# files are not overwritten when switching between builds.
ifeq ($(SANI),1)
	OBJ_DIR := $(BASE_OBJ_DIR)/sani
	NAME := $(NAME_SANI)
	UNIT := $(UNIT_SANI)
else
	OBJ_DIR := $(BASE_OBJ_DIR)/default
endif

# Subdirectory for header files
INC_DIR := inc

# Subdirectories for dependency files
DEP_DIR := $(BASE_OBJ_DIR)/dep

# Subdirectory for test related files
TEST_DIR := test

UNIT_TEST_DIR_SRC := $(TEST_DIR)/unit/src
UNIT_TEST_DIR_INC := $(TEST_DIR)/unit/inc
INTEGRATION_TEST_DIR := $(TEST_DIR)/integration
SIEGE_DIR := $(TEST_DIR)/siege
REQUESTER_DIR := $(TEST_DIR)/requester

# Subdirectory for log files
LOG_DIR := log

# Directory for coverage report
KCOV_DIR := .vscode/coverage

# Directory for configuration files
CONFIG_DIR := config_files

# ******************************
# *     Vars for compiling     *
# ******************************

CXX := c++
CPPFLAGS := -I $(INC_DIR)
WARNINGS := -Wall -Wextra -Werror -Wpedantic -Wdocumentation
CXXFLAGS = -std=c++98 $(WARNINGS) $(ASAN) $(DEBUG_FLAGS) -g
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEP_DIR)/$*.Td
LDFLAGS = $(ASAN)
LDLIBS =
COMPILE = $(CXX) $(DEPFLAGS) $(CPPFLAGS) $(CXXFLAGS) -c
POSTCOMPILE = @mv -f $(DEP_DIR)/$*.Td $(DEP_DIR)/$*.d && touch $@

# Special variables for compiling test files
CPPFLAGS_TEST = $(CPPFLAGS) -I $(UNIT_TEST_DIR_INC)
CXXFLAGS_TEST = -std=c++20 $(WARNINGS) $(ASAN) $(DEBUG_FLAGS) -g
COMPILE_TEST = $(CXX) $(DEPFLAGS) $(CPPFLAGS_TEST) $(CXXFLAGS_TEST) -c

# ******************************
# *     Source files           *
# ******************************

SRC:=	main.cpp \
		ALogOutputter.cpp \
		AutoindexHandler.cpp \
		CGIHandler.cpp \
		ConfigFile.cpp \
		ConfigFileParser.cpp \
		Connection.cpp \
		constants.cpp \
		DeleteHandler.cpp \
		Directory.cpp \
		EpollWrapper.cpp \
		FileSystemOps.cpp \
		FileWriteHandler.cpp \
		HTTPRequest.cpp \
		LogData.cpp \
		Logger.cpp \
		LogInit.cpp \
		LogOutputterConsole.cpp \
		LogOutputterFile.cpp \
		LogOstreamInserters.cpp \
		ProcessOps.cpp \
		RequestParser.cpp \
		ResponseBodyHandler.cpp \
		ResponseBuilder.cpp \
		Server.cpp \
		signalHandler.cpp \
		SocketOps.cpp \
		StatusCode.cpp \
		TargetResourceHandler.cpp \
		utilities.cpp

# ******************************
# *     Test source files      *
# ******************************

TEST_SRC :=	test_acceptConnections.cpp \
			test_AutoindexHandler.cpp \
			test_CGIHandler.cpp \
			test_checkForTimeout.cpp \
			test_ConfigFileParser.cpp \
			test_connectionBuildResponse.cpp \
			test_connectionHandleTimeout.cpp \
			test_connectionReceiveBody.cpp \
			test_connectionReceiveFromCGI.cpp \
			test_connectionReceiveHeader.cpp \
			test_connectionSendResponse.cpp \
			test_connectionSendToCGI.cpp \
			test_createVirtualServer.cpp \
			test_DeleteHandler.cpp \
			test_FileWriteHandler.cpp \
			test_handleBody.cpp \
			test_handleCompleteRequestHeader.cpp \
			test_handleEvent.cpp \
			test_helpers.cpp \
			test_initVirtualServers.cpp \
			test_isDuplicateServer.cpp \
			test_MultipartFormdata.cpp \
			test_OstreamInserters.cpp \
			test_parseBody.cpp \
			test_parseHeader_Headers.cpp \
			test_parseHeader_RequestLine.cpp \
			test_registerCGIFileDescriptor.cpp \
			test_registerConnection.cpp \
			test_registerVirtualServer.cpp \
			test_ResponseBodyHandler.cpp \
			test_ResponseBuilder.cpp \
			test_selectServerConfig.cpp \
			test_SocketOps_retrieveSocketInfo.cpp \
			test_shutdownServer.cpp \
			test_TargetResourceHandler.cpp \
			test_utils.cpp

# ******************************
# *     Object files           *
# ******************************

PROG_OBJS := $(addprefix $(OBJ_DIR)/, $(SRC:.cpp=.o))

# All tests files + program files. Filters out main.o from PROG_OBJS to use unittest main
TEST_OBJS :=	$(addprefix $(OBJ_DIR)/, $(TEST_SRC:.cpp=.o)) \
				$(filter-out $(OBJ_DIR)/main.o, $(PROG_OBJS))

# OBJS is defaulted to PROG_OBJS
OBJS := $(PROG_OBJS)

# ******************************
# *     Dependency files       *
# ******************************

DEPFILES =	$(SRC:%.cpp=$(DEP_DIR)/%.d)

DEPFILES +=	$(TEST_SRC:%.cpp=$(DEP_DIR)/%.d)

# ******************************
# *     Log files              *
# ******************************

LOG_FILE = $(LOG_DIR)/$(shell date "+%Y-%m-%d-%H-%M-%S")
LOG_VALGRIND = $(LOG_FILE)_valgrind.log
LOG_PERF = $(LOG_FILE)_perf.data
LOG_SIEGE = $(LOG_DIR)/siege.log
LOG_WEBSERV = /dev/null
#LOG_WEBSERV = $(LOG_FILE)_webserv.log

# ******************************
# *     Special Vars           *
# ******************************

CONFIGFILE = $(CONFIG_DIR)/example.conf
KCOV_EXCL_PATH = --exclude-path=/usr/include,/usr/lib,/usr/local,./$(TEST_DIR)

# ******************************
# *     Default target        *
# ******************************

.PHONY: all
all: $(NAME)

# ******************************
# *     Link target            *
# ******************************

$(NAME): $(PROG_OBJS)
	@printf "$(YELLOW)$(BOLD)link binary$(RESET) [$(BLUE)$@$(RESET)]\n"
	$(SILENT)$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@
	@printf "$(YELLOW)$(BOLD)compilation successful$(RESET) [$(BLUE)$@$(RESET)]\n"
	@printf "$(BOLD)$(GREEN)$@ created!$(RESET)\n"

# ******************************
# *     Special targets        *
# ******************************

# Reconfigure flags for linking with gtest
$(UNIT): LDLIBS := -pthread -lgtest -lgmock -lgtest_main
$(UNIT): OBJS := $(TEST_OBJS)
# Link the test binary
$(UNIT): $(TEST_OBJS)
	@printf "$(YELLOW)$(BOLD)link binary$(RESET) [$(BLUE)$@$(RESET)]\n"
	$(SILENT)$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@
	@printf "$(YELLOW)$(BOLD)compilation successful$(RESET) [$(BLUE)$@$(RESET)]\n"
	@printf "$(BOLD)$(GREEN)$@ created!$(RESET)\n"

# Run unittests
.PHONY: test
test: $(UNIT)
	@printf "$(YELLOW)$(BOLD)Run unittests$(RESET) [$(BLUE)$@$(RESET)]\n"
	$(SILENT)./$(UNIT)

# Run integration tests

PYTEST = -v -m "not timeout"
.PHONY: test2
test2: $(NAME)
	@printf "$(YELLOW)$(BOLD)Run integration tests$(RESET) [$(BLUE)$@$(RESET)]\n"
	$(SILENT)pytest $(PYTEST) \
	--server-executable=./$(NAME) \
	--config-file=./$(CONFIGFILE) \
	./$(INTEGRATION_TEST_DIR)

SIEGE_CONFIG=$(SIEGE_DIR)/siege.conf
SIEGE_FILE=$(SIEGE_DIR)/urls.txt
SIEGE_CONCURRENT=25
SIEGE_TIME=1m
SIEGE_URL=http://127.0.0.1:8080/empty.html

# Run load test with siege
.PHONY: test3
test3: $(NAME) | $(LOG_DIR)
	@printf "$(YELLOW)$(BOLD)Run load test with siege$(RESET) [$(BLUE)$@$(RESET)]\n"
	$(SILENT)./webserv $(CONFIGFILE) >$(LOG_WEBSERV) 2>&1 & echo $$! > webserv.pid
	$(SILENT)sleep 1
	$(SILENT)siege \
		--rc=$(SIEGE_CONFIG) \
		--file=$(SIEGE_FILE) \
		--log=$(LOG_SIEGE) \
		--concurrent=$(SIEGE_CONCURRENT) \
		--time=$(SIEGE_TIME)
	$(SILENT)kill `cat webserv.pid` && rm -f webserv.pid

.PHONY: test4
test4: $(NAME) | $(LOG_DIR)
	@printf "$(YELLOW)$(BOLD)Run benchmark test with siege on a single URL$(RESET) [$(BLUE)$@$(RESET)]\n"
	$(SILENT)./webserv $(CONFIGFILE) >$(LOG_WEBSERV) 2>&1 & echo $$! > webserv.pid
	$(SILENT)sleep 1
	$(SILENT)siege \
		--rc=$(SIEGE_CONFIG) \
		--log=$(LOG_SIEGE) \
		--concurrent=$(SIEGE_CONCURRENT) \
		--time=$(SIEGE_TIME) \
		--benchmark \
		$(SIEGE_URL)
	$(SILENT)kill `cat webserv.pid` && rm -f webserv.pid

REQUESTER=$(REQUESTER_DIR)/requester.py
REQUESTER_FILE=$(REQUESTER_DIR)/requests.txt
VALGRIND_FLAGS=--leak-check=full --show-leak-kinds=all --track-fds=yes --track-origins=yes --log-file=$(LOG_VALGRIND)

.PHONY: test5
test5: $(NAME) | $(LOG_DIR)
	@printf "$(YELLOW)$(BOLD)Run webserv with valgrind and requester$(RESET) [$(BLUE)$@$(RESET)]\n"
	$(SILENT)valgrind $(VALGRIND_FLAGS) ./webserv $(CONFIGFILE) >$(LOG_WEBSERV) 2>&1 & echo $$! > webserv.pid
	$(SILENT)sleep 2
	$(SILENT)/usr/bin/python3 \
		$(REQUESTER) \
		$(REQUESTER_FILE)
	$(SILENT)kill `cat webserv.pid` && rm -f webserv.pid
	$(SILENT)less $(LOG_VALGRIND)

# This target uses perf for profiling.
.PHONY: profile
profile: check_perf_installed $(NAME) | $(LOG_DIR)
	@printf "$(YELLOW)$(BOLD)Run for profiling with perf$(RESET) [$(BLUE)$@$(RESET)]\n"
	$(eval NEW_LOG_FILE=$(LOG_PERF))
	$(SILENT)perf record -g -o $(NEW_LOG_FILE) ./$(NAME)
	@printf "$(YELLOW)$(BOLD)Saved log file$(RESET) [$(BLUE)$@$(RESET)]\n"
	@printf "$(NEW_LOG_FILE)\n"
	@printf "$(YELLOW)$(BOLD)Run perf report$(RESET) [$(BLUE)$@$(RESET)]\n"
	$(SILENT)perf report -g -i $(NEW_LOG_FILE)

# Check if perf is installed. If not exit with error.
.PHONY: check_perf_installed
check_perf_installed:
	@command -v perf >/dev/null 2>&1 || { \
		echo >&2 "perf is not installed. Please install perf to continue."; exit 1; \
	}

# This target creates compile_commands.json for clangd.
.PHONY: comp
comp: check_bear_installed clean
	@printf "$(YELLOW)$(BOLD)Creating compile_commands.json$(RESET) [$(BLUE)$@$(RESET)]\n"
	$(SILENT)bear -- make -j4 --no-print-directory $(UNIT)

# Check if bear is installed. If not exit with error.
.PHONY: check_bear_installed
check_bear_installed:
	@command -v bear >/dev/null 2>&1 || { \
		echo >&2 "bear is not installed. Please install bear to continue."; exit 1; \
	}

# Create coverage report to display with coverage gutter
.PHONY: coverage
coverage: $(UNIT) | $(KCOV_DIR)
	@printf "$(YELLOW)$(BOLD)Creating coverage report from $(UNIT)$(RESET) [$(BLUE)$@$(RESET)]\n"
	$(SILENT)kcov $(KCOV_EXCL_PATH) $(KCOV_DIR) ./$(UNIT)

.PHONY: coverage2
coverage2: $(NAME) | $(KCOV_DIR)
	@printf "$(YELLOW)$(BOLD)Creating coverage report from integration tests$(RESET) [$(BLUE)$@$(RESET)]\n"
	$(SILENT)pytest \
	--server-executable=./$(NAME) \
	--config-file=./$(CONFIGFILE) \
	--with-coverage \
	--kcov-output-dir=$(KCOV_DIR) \
	--kcov-excl-path=$(KCOV_EXCL_PATH) \
	./$(INTEGRATION_TEST_DIR)

# ******************************
# *     Object compiling and   *
# *     dependecy creation     *
# ******************************

# File counter for status output
TOTAL_FILES = $(words $(OBJS))
CURRENT_FILE := 0

# Create object and dependency files
# $(DEP_DIR)/%.d =	Declare the generated dependency file as a prerequisite of the target,
# 					so that if it’s missing the target will be rebuilt.
# | $(DEPDIR) = 	Declare the dependency directory as an order-only prerequisite of the target,
# 					so that it will be created when needed.
# $(eval ...) =		Increment file counter.
# $(POSTCOMPILE) =	Move temp dependency file and touch object to ensure right timestamps.

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp message $(DEP_DIR)/%.d | $(DEP_DIR) $(OBJ_DIR)
	$(eval CURRENT_FILE=$(shell echo $$(($(CURRENT_FILE) + 1))))
	@echo "($(CURRENT_FILE)/$(TOTAL_FILES)) Compiling $(BOLD)$< $(RESET)"
	$(SILENT)$(COMPILE) $< -o $@
	$(SILENT)$(POSTCOMPILE)

# Similar target for testfiles; uses different compile flags
$(OBJ_DIR)/%.o: $(UNIT_TEST_DIR_SRC)/%.cpp message $(DEP_DIR)/%.d | $(DEP_DIR) $(OBJ_DIR)
	$(eval CURRENT_FILE=$(shell echo $$(($(CURRENT_FILE) + 1))))
	@echo "($(CURRENT_FILE)/$(TOTAL_FILES)) Compiling $(BOLD)$< $(RESET)"
	$(SILENT)$(COMPILE_TEST) $< -o $@
	$(SILENT)$(POSTCOMPILE)

# Print message only if there are objects to compile
.INTERMEDIATE: message
message:
	@printf "$(YELLOW)$(BOLD)compile objects$(RESET) [$(BLUE)$@$(RESET)]\n"

# Create subdirectory if it doesn't exist
$(DEP_DIR) $(LOG_DIR) $(KCOV_DIR) $(OBJ_DIR):
	@printf "$(YELLOW)$(BOLD)create subdir$(RESET) [$(BLUE)$@$(RESET)]\n"
	@echo $@
	$(SILENT)mkdir -p $@

# Mention each dependency file as a target, so that make won’t fail if the file doesn’t exist.
$(DEPFILES):

# ******************************
# *     Cleanup                *
# ******************************

# Remove all object files and dependency files
.PHONY: clean
clean:
	@printf "$(YELLOW)$(BOLD)clean$(RESET) [$(BLUE)$@$(RESET)]\n"
	@rm -rf $(BASE_OBJ_DIR)
	@printf "$(RED)removed directory $(BASE_OBJ_DIR)$(RESET)\n"

# Remove all object, dependency, binaries and log files
.PHONY: fclean
fclean: clean
	@rm -rf $(NAME) $(UNIT) $(NAME_SANI) $(UNIT_SANI)
	@printf "$(RED)removed binaries $(NAME)* $(UNIT)*$(RESET)\n"
	@rm -rf $(LOG_DIR)
	@printf "$(RED)removed subdir $(LOG_DIR)$(RESET)\n"
	@rm -rf $(KCOV_DIR)
	@printf "$(RED)removed subdir $(KCOV_DIR)$(RESET)\n"
	@rm -rf $(INTEGRATION_TEST_DIR)/__pycache__ \
			$(INTEGRATION_TEST_DIR)/*/__pycache__ \
			$(INTEGRATION_TEST_DIR)/.pytest_cache
	@printf "$(RED)removed .pytest_cache and directories __pychache__$(RESET)\n"
	@rm -f webserv.pid
	@printf "$(RED)removed webserv.pid$(RESET)\n"
	@rm -f compile_commands.json
	@printf "$(RED)removed compile_commands.json$(RESET)\n"
	@echo

# ******************************
# *     Recompilation          *
# ******************************

.PHONY: re
re: fclean all

# ******************************
# *     Various                *
# ******************************

# Include the dependency files that exist. Use wildcard to avoid failing on non-existent files.
# Needs to be last target
include $(wildcard $(DEPFILES))

# ******************************
# *     Help Target            *
# ******************************

.PHONY: help
help:
	@echo "$(GREEN)$(BOLD)$(NAME) Makefile Help$(RESET)"
	@echo "This Makefile automates the compilation and cleaning processes for $(NAME)."
	@echo "It supports various targets and can be customized with different variables."
	@echo "Below are the available targets and variables you can use."
	@echo ""
	@echo "$(YELLOW)Targets:$(RESET)"
	@echo "  all         - Compiles the default version of the $(NAME) program."
	@echo "  test        - Compiles the unit tests linking with gtest and gmock."
	@echo "  clean       - Removes object files and dependency files."
	@echo "  fclean      - Performs 'clean' and also removes binaries and log files."
	@echo "  re          - Performs 'fclean' and then 'all'."
	@echo "  valgr       - Runs the program with Valgrind to check for memory leaks."
	@echo "  profile     - Profiles the program using 'perf'."
	@echo "  comp        - Creates compile_commands.json for clangd."
	@echo ""
	@echo "$(YELLOW)Variables:$(RESET)"
	@echo "  VERBOSE=1   - Echoes all commands if set to 1."
	@echo "  SANI=1      - Enables sanitizers if set to 1."
	@echo ""
	@echo "$(YELLOW)Examples:$(RESET)"
	@echo "  make all VERBOSE=1   - Compiles the default target with command echoing."
	@echo ""
	@echo "For more detailed information, read the comments within the Makefile itself."
