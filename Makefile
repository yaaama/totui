##
# Totui
#
# @file
# @version 1.0

# Source files
SRCS = $(wildcard src/*.c)

# Object files
OBJS = $(addprefix build/,$(notdir $(SRCS:.c=.o)))

# Flags to use for development
CFLAGS = -Wall -g3 -O0 -std=c11 -Wextra -fanalyzer -Wunused -pedantic -lncurses -flto=auto -lformw -lmenuw -ldialog -lncursesw -lm

# CFLAGS to use for users
# CFLAGS = -Wall -std=c11 -Wextra -ldialog -lncursesw -lm

# specify your compiler
CC = gcc

# default target
all: build/totui

# Developer builds
build/totui: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

build/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@


# Cleans build directory
clean:
	rm -f build/*

# a target to build the program using bear (for dev use only)
build: clean
	bear -- make all

# Builds then moves the executable to the parent directory
install: build/totui
	mv ./build/totui ../totui

# a target to run program
run: build/totui
	./build/totui

#end
