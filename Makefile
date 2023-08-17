##
# Totui
#
# @file
# @version 0.1

SRCS = $(wildcard src/*.c)

# specify your object files here
OBJS = $(addprefix build/,$(notdir $(SRCS:.c=.o)))

# specify your compiler flags here
CFLAGS = -Wall -g3 -O0 -std=c11 -Wextra -pedantic -lncurses -flto=auto -lformw -lmenuw -ldialog -lncursesw -lm

# specify your compiler
CC = gcc


compile_commands.json: clean
	bear -- make all

# default command to compile your program
all: build/totui

build/totui: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

build/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# a command to clean up your build directory
clean:
	rm -f build/*

# a command to run your program
run: clean build/totui
	./build/totui


# end
