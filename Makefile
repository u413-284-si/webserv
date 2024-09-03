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
# *     Verbosity              *
# ******************************

# Set VERBOSE=1 to echo all commands
ifeq ($(VERBOSE),1)
	SILENT =
else
	SILENT = @
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
# *     Targets                *
# ******************************

NAME := webserv

TEST := unittest

TEST_SANI := unittest_sani

# ******************************
# *     Directories            *
# ******************************

SRC_DIR := src

TEST_DIR := test

# Base directory for object files
BASE_OBJ_DIR = obj

# If condition to check target and set object directory accordingly
ifeq ($(MAKECMDGOALS),test)
	OBJ_DIR := $(BASE_OBJ_DIR)/$(TEST)
else ifeq ($(MAKECMDGOALS),test_sani)
	OBJ_DIR := $(BASE_OBJ_DIR)/$(TEST_SANI)
else ifeq ($(MAKECMDGOALS),doxycheck)
	OBJ_DIR := $(BASE_OBJ_DIR)/doxycheck
else
	OBJ_DIR := $(BASE_OBJ_DIR)/$(NAME)
endif

# Subdirectory for header files
INC_DIR := inc

# Subdirectories for dependency files
DEP_DIR := $(OBJ_DIR)/dep

# Subdirectory for log files
LOG_DIR := log

# Directory for coverage report
COV_DIR := .vscode/coverage

# ******************************
# *     Vars for compiling     *
# ******************************

CXX := c++
CPPFLAGS := -I $(INC_DIR)
CXXFLAGS = -std=c++98 -Wall -Werror -Wextra -Wpedantic -g
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEP_DIR)/$*.Td
LDFLAGS =
LDLIBS =
COMPILE = $(CXX) $(DEPFLAGS) $(CPPFLAGS) $(CXXFLAGS) -c
POSTCOMPILE = @mv -f $(DEP_DIR)/$*.Td $(DEP_DIR)/$*.d && touch $@

# ******************************
# *     Source files           *
# ******************************

SRC:=	main.cpp \
		ALogOutputter.cpp \
		AutoindexHandler.cpp \
		ConfigFile.cpp \
		ConfigFileParser.cpp \
		Connection.cpp \
		Directory.cpp \
		EpollWrapper.cpp \
		FileSystemPolicy.cpp \
		LogData.cpp \
		Logger.cpp \
		LogInit.cpp \
		LogOutputterConsole.cpp \
		LogOutputterFile.cpp \
		LogOstreamInserters.cpp \
		RequestParser.cpp \
		ResponseBodyHandler.cpp \
		ResponseBuilder.cpp \
		Server.cpp \
		signalHandler.cpp \
		SocketPolicy.cpp \
		TargetResourceHandler.cpp \
		utilities.cpp

# ******************************
# *     Test source files      *
# ******************************

TEST_SRC :=	test_acceptConnections.cpp \
			test_AutoindexHandler.cpp \
			test_checkForTimeout.cpp \
			test_ConfigFileParser.cpp \
			test_connectionReceiveHeader.cpp \
			test_connectionSendResponse.cpp \
			test_createVirtualServer.cpp \
			test_initVirtualServers.cpp \
			test_isDuplicateServer.cpp \
			test_OstreamInserters.cpp \
			test_parseBody.cpp \
			test_parseHeader_Headers.cpp \
			test_parseHeader_RequestLine.cpp \
			test_registerConnection.cpp \
			test_registerVirtualServer.cpp \
			test_ResponseBodyHandler.cpp \
			test_serverShutdown.cpp \
			test_TargetResourceHandler.cpp \
			test_utils.cpp

# ******************************
# *     Object files           *
# ******************************

PROG_OBJS := $(addprefix $(OBJ_DIR)/, $(SRC:.cpp=.o))

TEST_OBJS :=	$(addprefix $(OBJ_DIR)/, $(TEST_SRC:.cpp=.o)) \
				$(filter-out $(OBJ_DIR)/main.o, $(PROG_OBJS))

# If condition to check target and set objects accordingly
ifeq ($(findstring test, $(MAKECMDGOALS)),test)
	OBJS := $(TEST_OBJS)
else
	OBJS := $(PROG_OBJS)
endif

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

# ******************************
# *     Default target        *
# ******************************

.PHONY: all
all: $(NAME)

# ******************************
# *     Link target            *
# ******************************

# Linking targets
# NAME
# TEST
# TEST_SANI
$(NAME) $(TEST) $(TEST_SANI): $(OBJS)
	@printf "$(YELLOW)$(BOLD)link binary$(RESET) [$(BLUE)$@$(RESET)]\n"
	$(SILENT)$(CXX) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@
	@printf "$(YELLOW)$(BOLD)compilation successful$(RESET) [$(BLUE)$@$(RESET)]\n"
	@printf "$(BOLD)$(GREEN)$@ created!$(RESET)\n"

# ******************************
# *     Special targets        *
# ******************************

# Alias for creating unittests
.PHONY: test
# Reconfigure flags for linking with gtest
test: CXXFLAGS = -Wall -Werror -Wextra -g
test: LDLIBS = -lpthread -lgtest -lgmock -lgtest_main
test: $(TEST)

# Alias for creating unittests with sanitizers enabled
.PHONY: test_sani
# Reconfigure flags for linking with gtest and sanitizers
test_sani: CXXFLAGS = -Wall -Werror -Wextra -g -fsanitize=address,undefined
test_sani: LDFLAGS = -fsanitize=address,undefined
test_sani: LDLIBS = -lpthread -lgtest -lgmock -lgtest_main
test_sani: $(TEST_SANI)

# This target uses the file valid_config.conf as argument to run the program.
.PHONY: run
run: $(NAME)
	@printf "$(YELLOW)$(BOLD)Run with standard_config.conf as argument$(RESET) [$(BLUE)$@$(RESET)]\n"
	./webserv config_files/standard_config.conf

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

# Perform memory check on NAME.
.PHONY: valgr
valgr: $(NAME) | $(LOG_DIR)
	$(SILENT)valgrind	--leak-check=full\
						--show-leak-kinds=all\
						--track-fds=yes\
						--log-file=$(LOG_VALGRIND)\
						./$(NAME)
	$(SILENT)ls -dt1 $(LOG_DIR)/* | head -n 1 | xargs less

# This target creates compile_commands.json for clangd.
.PHONY: comp
comp: check_bear_installed clean
	@printf "$(YELLOW)$(BOLD)Creating compile_commands.json$(RESET) [$(BLUE)$@$(RESET)]\n"
	$(SILENT)bear -- make -j --no-print-directory
	$(SILENT)bear --append -- make -j --no-print-directory test

# Check if bear is installed. If not exit with error.
.PHONY: check_bear_installed
check_bear_installed:
	@command -v bear >/dev/null 2>&1 || { \
		echo >&2 "bear is not installed. Please install bear to continue."; exit 1; \
	}

# Create coverage report to display with coverage gutter
EXCL_PATH = --exclude-path=/usr/include,/usr/lib,/usr/local,./$(TEST_DIR)
.PHONY: coverage
coverage: | $(COV_DIR)
	@printf "$(YELLOW)$(BOLD)Creating coverage report as index.html$(RESET) [$(BLUE)$@$(RESET)]\n"
	$(SILENT)kcov $(EXCL_PATH) $(COV_DIR) ./$(TEST)
	@printf "$(YELLOW)$(BOLD)Creating coverage report as cov.xml$(RESET) [$(BLUE)$@$(RESET)]\n"
	$(SILENT)kcov $(EXCL_PATH) --cobertura-only $(COV_DIR) ./$(TEST)

.PHONY: doxycheck
doxycheck: CXXFLAGS += -Wdocumentation
doxycheck: $(NAME)

# ******************************
# *     Object compiling and   *
# *     dependecy creation     *
# ******************************

# File counter for status output
TOTAL_FILES := $(words $(OBJS))
CURRENT_FILE := 0

# Create object and dependency files
# $(DEP_DIR)/%.d =	Declare the generated dependency file as a prerequisite of the target,
# 					so that if it’s missing the target will be rebuilt.
# | $(DEPDIR) = 	Declare the dependency directory as an order-only prerequisite of the target,
# 					so that it will be created when needed.
# $(eval ...) =		Increment file counter.
# $(POSTCOMPILE) =	Move temp dependency file and touch object to ensure right timestamps.

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp message $(DEP_DIR)/%.d | $(DEP_DIR)
	$(eval CURRENT_FILE=$(shell echo $$(($(CURRENT_FILE) + 1))))
	@echo "($(CURRENT_FILE)/$(TOTAL_FILES)) Compiling $(BOLD)$< $(RESET)"
	$(SILENT)$(COMPILE) $< -o $@
	$(SILENT)$(POSTCOMPILE)

# Similar target for testfiles
$(OBJ_DIR)/%.o: $(TEST_DIR)/%.cpp message $(DEP_DIR)/%.d | $(DEP_DIR)
	$(eval CURRENT_FILE=$(shell echo $$(($(CURRENT_FILE) + 1))))
	@echo "($(CURRENT_FILE)/$(TOTAL_FILES)) Compiling $(BOLD)$< $(RESET)"
	$(SILENT)$(COMPILE) $< -o $@
	$(SILENT)$(POSTCOMPILE)

# Print message only if there are objects to compile
.INTERMEDIATE: message
message:
	@printf "$(YELLOW)$(BOLD)compile objects$(RESET) [$(BLUE)$@$(RESET)]\n"

# Create subdirectory if it doesn't exist
$(DEP_DIR) $(LOG_DIR) $(COV_DIR):
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
	@rm -rf $(NAME) $(TEST) $(TEST_SANI)
	@printf "$(RED)removed binaries $(NAME) $(TEST) $(TEST_SANI)$(RESET)\n"
	@rm -rf $(LOG_DIR)
	@printf "$(RED)removed subdir $(LOG_DIR)$(RESET)\n"
	@rm -rf $(COV_DIR)
	@printf "$(RED)removed subdir $(COV_DIR)$(RESET)\n"
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
	@echo ""
	@echo "$(YELLOW)Examples:$(RESET)"
	@echo "  make all VERBOSE=1   - Compiles the default target with command echoing."
	@echo ""
	@echo "For more detailed information, read the comments within the Makefile itself."
