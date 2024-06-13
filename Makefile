# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: gwolf <gwolf@student.42vienna.com>         +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/09/07 19:36:40 by gwolf             #+#    #+#              #
#    Updated: 2024/06/13 12:22:19 by gwolf            ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME := a.out

CXX := c++
CXXFLAGS = -Wall -Werror -Wextra --std=c++98 -I include
RM := rm -fr

SRCDIR := src
OBJDIR := obj

SRC =	LogData.cpp \
		Logger.cpp \
		LogOutputterConsole.cpp \
		LogOutputterFile.cpp \
		main.cpp

OBJ = $(addprefix $(OBJDIR)/, $(SRC:.cpp=.o))

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	$(RM) $(OBJDIR)/

fclean: clean
	$(RM) $(NAME)

re: fclean all