# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: sqiu <sqiu@student.42vienna.com>           +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/07/28 13:03:05 by gwolf             #+#    #+#              #
#    Updated: 2024/06/05 23:52:30 by sqiu             ###   ########.fr        #
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
# *     Directories            *
# ******************************

SRC_DIR := src

TEST_DIR := test

# Base directory for object files
OBJ_DIR := obj

# Subdirectory for header files
INC_DIR := inc

# Subdirectories for dependency files
DEP_DIR := $(OBJ_DIR)/dep

# Subdirectory for log files
LOG_DIR := log

# ******************************
# *     Vars for compiling     *
# ******************************

CXX := c++
CPPFLAGS := -I $(INC_DIR)
CXXFLAGS = -Wall -Werror -Wextra -std=c++98 -pedantic
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEP_DIR)/$*.Td
COMPILE = $(CXX) $(DEPFLAGS) $(CPPFLAGS) $(CXXFLAGS) -c
POSTCOMPILE = @mv -f $(DEP_DIR)/$*.Td $(DEP_DIR)/$*.d && touch $@

# ******************************
# *     Targets                *
# ******************************

# Target
NAME := webserv
TEST := unittest

# ******************************
# *     Source files           *
# ******************************

SRC:=	main.cpp \
		FileHandler.cpp \
		ResponseBuilder.cpp \
		Server.cpp \
		TargetResourceHandler.cpp

# ******************************
# *     Test source files      *
# ******************************

TEST_SRC := test_TargetResourceHandler.cpp

# ******************************
# *     Object files           *
# ******************************

OBJS = 	$(addprefix $(OBJ_DIR)/, $(SRC:.cpp=.o))
TEST_OBJS = $(addprefix $(TEST_DIR)/, $(TEST_SRC:.cpp=.o))
TEST_OBJS += $(filter-out $(OBJ_DIR)/main.o, $(OBJS))

# ******************************
# *     Dependency files       *
# ******************************

DEPFILES =	$(SRC:%.cpp=$(DEP_DIR)/%.d)

# ******************************
# *     Log files              *
# ******************************

LOG_FILE = $(LOG_DIR)/$(shell date "+%Y-%m-%d-%H-%M-%S")
LOG_VALGRIND = $(LOG_FILE)_valgrind.log
LOG_PERF = $(LOG_FILE)_perf.data

# ******************************
# *     Default target         *
# ******************************

.PHONY: all
all: $(NAME)

# ******************************
# *     NAME linkage           *
# ******************************

# Linking the NAME target
$(NAME): $(OBJS)
	@printf "$(YELLOW)$(BOLD)link binary$(RESET) [$(BLUE)$@$(RESET)]\n"
	$(SILENT)$(CXX) $(OBJS) -o $@
	@printf "$(YELLOW)$(BOLD)compilation successful$(RESET) [$(BLUE)$@$(RESET)]\n"
	@printf "$(BOLD)$(GREEN)$(NAME) created!$(RESET)\n"

# ******************************
# *     Special targets        *
# ******************************

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

.PHONY: test
test: CXXFLAGS = -Wall -Werror
test: $(TEST_OBJS)
	@printf "$(YELLOW)$(BOLD)link $(TEST)$(RESET) [$(BLUE)$@$(RESET)]\n"
	$(SILENT)$(CXX) $(TEST_OBJS) -lgtest -lgmock -lgmock_main -o $(TEST)
	@printf "$(YELLOW)$(BOLD)compilation successful$(RESET) [$(BLUE)$@$(RESET)]\n"
	@printf "$(BOLD)$(GREEN)$(TEST) created!$(RESET)\n"

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
# $(eval ...) =		Increment file counter.
# $(POSTCOMPILE) =	Move temp dependency file and touch object to ensure right timestamps.

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp message $(DEP_DIR)/%.d | $(DEP_DIR)
	$(eval CURRENT_FILE=$(shell echo $$(($(CURRENT_FILE) + 1))))
	@echo "($(CURRENT_FILE)/$(TOTAL_FILES)) Compiling $(BOLD)$< $(RESET)"
	$(SILENT)$(COMPILE) $< -o $@
	$(SILENT)$(POSTCOMPILE)

# Print message only if there are objects to compile
.INTERMEDIATE: message
message:
	@printf "$(YELLOW)$(BOLD)compile objects$(RESET) [$(BLUE)$@$(RESET)]\n"

# Create subdirectory if it doesn't exist
$(DEP_DIR) $(LOG_DIR):
	@printf "$(YELLOW)$(BOLD)create subdir$(RESET) [$(BLUE)$@$(RESET)]\n"
	@echo $@
	$(SILENT)mkdir -p $@

# Mention each dependency file as a target, so that make won’t fail if the file doesn’t exist.
$(DEPFILES):

$(TEST_DIR)/%.o: $(TEST_DIR)/%.cpp test_message
	$(eval CURRENT_FILE=$(shell echo $$(($(CURRENT_FILE) + 1))))
	@echo "($(CURRENT_FILE)/$(TOTAL_FILES)) Compiling $(BOLD)$< $(RESET)"
	$(SILENT)$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

# Print message only if there are objects to compile
.INTERMEDIATE: test_message
test_message:
	@printf "$(YELLOW)$(BOLD)compile test objects$(RESET) [$(BLUE)$@$(RESET)]\n"

# ******************************
# *     Cleanup                *
# ******************************

# Remove all object files and dependency files
.PHONY: clean
clean:
	@printf "$(YELLOW)$(BOLD)clean$(RESET) [$(BLUE)$@$(RESET)]\n"
	@rm -rf $(OBJ_DIR) $(TEST_DIR)/*.o
	@printf "$(RED)removed dir $(OBJ_DIR)$(RESET)\n"
	@printf "$(RED)removed *.o in dir $(TEST_DIR)$(RESET)\n"

# Remove all object, dependency, binaries and log files
.PHONY: fclean
fclean: clean
	@rm -rf $(NAME)*
	@printf "$(RED)removed binaries $(NAME)*$(RESET)\n"
	@rm -rf $(LOG_DIR)
	@printf "$(RED)removed subdir $(LOG_DIR)$(RESET)\n"
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
	@echo "  all         - Compiles the default version of the miniRT program."
	@echo "  clean       - Removes object files and dependency files."
	@echo "  fclean      - Performs 'clean' and also removes binaries and log files."
	@echo "  re          - Performs 'fclean' and then 'all'."
	@echo "  valgr       - Runs the program with Valgrind to check for memory leaks."
	@echo "  profile     - Profiles the program using 'perf'."
	@echo ""
	@echo "$(YELLOW)Variables:$(RESET)"
	@echo "  VERBOSE=1   - Echoes all commands if set to 1."
	@echo ""
	@echo "$(YELLOW)Examples:$(RESET)"
	@echo "  make all VERBOSE=1   - Compiles the default target with command echoing."
	@echo ""
	@echo "For more detailed information, read the comments within the Makefile itself."
