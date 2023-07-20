#include "item.h"
#include <curses.h>
#include <form.h>
#include <menu.h>
#include <ncurses.h>
#include <panel.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define PADDING_X 5
#define PADDING_Y 3
#define MAX_TODO_LEN 64

/* Keeps a track of the current line number the user is on */
unsigned int curr_line = PADDING_Y;

/* list is 64 characters for a single todo header and 16 headers allowed max */
char todoList[128][64];
int currItem = 0;

void todo_create(WINDOW *todoWin, char *str);

void print_msg(int code) {
  switch (code) {
  case 0:
    printf("Bye bye friend!\n");
    exit(0);
    break;
  case 1:
    perror("Buffer overflow. Input is too big\n");
    exit(1);
    break;
  }
}

void verify_input(char *inp) {

  if (strlen(inp) > MAX_TODO_LEN) {
    print_msg(1);
  } else if (strlen(inp) == 1) {

    /* User has requested to quit */
    if (*inp == 'q' || *inp == 'Q') {
      print_msg(0);
    }
  }
}

void todo_create(WINDOW *todoWin, char *str) {

  strcpy(todoList[currItem], str);
  /* printf("%s has been added to the todo list!\n", str); */
  currItem++;
  mvwprintw(todoWin, curr_line, PADDING_X, "[ ]    %s", str);
  curr_line++;
  wrefresh(todoWin);
}

void initialise_window(void) {

  WINDOW *mainWin = initscr();
  clear();

  WINDOW *todoWin = newwin(LINES, COLS, 0, 0);
  wborder(todoWin, 0, 0, 0, 0, 0, 0, 0, 0);
  wattron(todoWin, A_STANDOUT);
  wprintw(todoWin, "Enter q/Q to exit.");
  wrefresh(todoWin);
  wattroff(todoWin, A_STANDOUT);

  for (int index = 0; index < 10; index++) {
    todo_create(todoWin, "item ");
  }

  mvwvline(todoWin, curr_line, PADDING_X, 0, 0);
  wrefresh(todoWin);

  char inp[64];

  while (true) {
    wgetstr(todoWin, inp);
    clrtobot();
    verify_input(inp);

    todo_create(todoWin, inp);

    mvwvline(todoWin, curr_line, PADDING_X, 0, 0);
    wrefresh(todoWin);
  }
}

int main(void) {

  initialise_window();

  return 0;
}
