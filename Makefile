##
# Totui
#
# @file
# @version 0.1

# specify your source files here
SRCS = $(wildcard *.c)

# specify your object files here
OBJS = $(addprefix build/,$(notdir $(SRCS:.c=.o)))

# specify your compiler flags here
CFLAGS = -Wall -ggdb -std=c11 -Wextra -pedantic -lncurses -lform -lmenu

# specify your compiler
CC = clang

# default command to compile your program
all: build/totui

build/totui: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

build/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# a command to clean up your build directory
clean:
	rm -f build/*

# a command to run your program
run: build/totui
	./build/totui

# end
