#include "item.h"
#include <ncurses.h>
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
    endwin();
    clear();
    exit(0);
    break;
  case 1:

    perror("Buffer overflow. Input is too big\n");
    endwin();
    exit(1);
    break;
  }
}

void todo_create(WINDOW *todoWin, char *str) {

  strcpy(todoList[currItem], str);
  /* printf("%s has been added to the todo list!\n", str); */
  currItem++;
  mvwprintw(todoWin, curr_line, PADDING_X, "[ ]    %s", str);
  curr_line++;
  /* wrefresh(todoWin); */
}

void verify_input(char *inp) {

  if (*inp == 9) {
  }

  if (strlen(inp) > MAX_TODO_LEN) {
    print_msg(1);
  } else if (strlen(inp) == 1) {

    /* User has requested to quit */
    if (*inp == 'q' || *inp == 'Q') {
      print_msg(0);
    }
  }
}

void initialise_window(void) {

  initscr();
  clear();
  nonl();

  WINDOW *todoWin = newwin(LINES, COLS / 2, 0, 0);
  wborder(todoWin, 0, 0, 0, 0, 0, 0, 2, 0);
  WINDOW *infoWin = newwin(LINES, COLS / 2, 0, COLS / 2);
  wborder(infoWin, 0, 0, 0, 0, 0, 0, 2, 0);

  /* Colours initialisation */
  start_color();
  init_pair(1, COLOR_RED, COLOR_RED);
  init_pair(2, COLOR_GREEN, COLOR_BLACK);
  init_pair(3, COLOR_BLUE, COLOR_BLACK);
  init_pair(4, COLOR_CYAN, COLOR_BLACK);

  /* Printing stuff to the top */
  wattron(todoWin, A_STANDOUT);
  wattron(todoWin, COLOR_PAIR(3));
  wattron(infoWin, A_STANDOUT);
  wattron(infoWin, COLOR_PAIR(3));

  wprintw(todoWin, "Enter q/Q to exit.");
  wprintw(infoWin, "Extra information");

  wattroff(todoWin, A_STANDOUT);
  wattroff(todoWin, COLOR_PAIR(3));
  wattroff(infoWin, COLOR_PAIR(3));
  wattroff(infoWin, A_STANDOUT);


  /* Creating fake todo list */
  for (int index = 0; index < 10; index++) {
    todo_create(todoWin, "item ");
  }
  mvwvline(todoWin, curr_line, PADDING_X, 0, 0);

  wrefresh(infoWin);
  wrefresh(todoWin);

  /* Input that the user will enter */
  char inp[64];

  char key;

  /* wrefresh(todoWin); */
  /* wrefresh(infoWin); */
  while (true) {
    wgetstr(todoWin, inp);
    /* clrtobot(); */

    wprintw(infoWin, "User entered %s\n", inp);

    if (inp[0] == 'g') {
      /* bottom_panel(NULL); */
      mvcur(getcurx(todoWin), getcury(todoWin), curr_line, COLS / 2);
      wrefresh(todoWin);
      wrefresh(infoWin);
      doupdate();
    }
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
