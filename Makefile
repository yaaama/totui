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

# default target
all: build/totui

build/totui: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

build/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# a target to clean up your build directory
clean:
	rm -f build/*

# a target to build the program
build: clean
	bear -- make all

# a target to run your program
run: build/totui
	./build/totui

#end
