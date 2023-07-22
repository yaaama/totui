#include "item.h"
#include <assert.h>
#include <curses.h>
#include <dialog.h>
#include <ncurses.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define _GNU_SOURCE

#define PADDING_X 5
#define PADDING_Y 3
#define MAX_TODO_LEN 64

/*******************/
/* /\* Typedef *\/ */
/*******************/
void append_to_file(char *file, char *str);
void new_todo_handler(void);
void refresh_screens(void);
void init_todo_from_file(char *file);
void init_display_items_todo_window(void);


typedef struct {
  char text[128];
} Item;

typedef struct {
  size_t size;
  Item items[128];
} All_Items;

typedef struct {
  bool complete;
  bool selected;
  Item *itemPtr;
  size_t index;
} Todo_Window_Cell;

typedef struct {
  WINDOW *window;
  Todo_Window_Cell *currentCell;
  size_t currentCellIndex;
  size_t totalCells;
} Todo_Window;

All_Items items = {0};

WINDOW *todoWin;
Todo_Window todoPane;

WINDOW *infoWin;

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

/* Defining colours */
void init_colour_pairs(void) {
  init_pair(1, COLOR_RED, COLOR_RED);
  init_pair(2, COLOR_GREEN, COLOR_BLACK);
  init_pair(3, COLOR_BLUE, COLOR_BLACK);
  init_pair(4, COLOR_CYAN, COLOR_BLACK);
}

void todo_window_loop(void) {

  char key;

  while (true) {
    key = wgetch(todoPane.window);

    if (key == 'q' || key == 'Q') {
      /* Exit program here */
    }

    /* Remove later */
    wprintw(infoWin, "User entered %c\n", key);
    wrefresh(infoWin);

    if (key == 'a') {
      /* TODO Menu to ask them what to do */
      new_todo_handler();
      refresh_screens();


    } else {
    }
  }
}


void refresh_screens(void) {

  wrefresh(todoPane.window);
  wrefresh(infoWin);
  redrawwin(todoPane.window);
  redrawwin(infoWin);

}



void new_todo_handler(void) {

  dialog_inputbox("New todo!", "Enter new item:", 20, 50, "", 0);
  dialog_vars.dlg_clear_screen = true;
  end_dialog();
  char *inp = dialog_vars.input_result;

  if (strlen(inp) == 0 || inp == NULL) {
  perror("Empty input.");
  dlg_clr_result();
  refresh_screens();

  return;
  } else {
  append_to_file("example.txt", inp);

                                /* Adds the new item to the todo items list */
  Item t;
  strncpy(t.text, inp, 127);
  items.items[items.size] = t;
  items.size ++;

  init_display_items_todo_window();
  dlg_clr_result();
  refresh_screens();

  /* todoPane.totalCells ++; */
  }


}




void append_to_file(char *file, char *str) {

  FILE *fp = fopen(file, "a");

  assert(fp != NULL);

  fprintf(fp, "%s\n", str);

  assert(str != NULL);
  fclose(fp);
}

void init_todo_from_file(char *file) {

  FILE *fp = fopen(file, "r+");
  assert(fp != NULL);
  char currLine[128];

  /* TODO Check if filename is real */

  size_t index = 0;

  while (fgets(currLine, MAX_TODO_LEN, fp) != NULL) {

    /* Creating Item structs for each line */
    Item todoItem;
    strncpy(todoItem.text, currLine, 127);

    items.items[index] = todoItem; /* Adding item to the global array */
    items.size++;
    index++;
  }

  fclose(fp);
}

/* Displaying the loaded list into the todo window */
void init_display_items_todo_window(void) {

  todoPane.totalCells = 0;

  for (size_t i = 0; i < items.size; i++) {
    wprintw(todoPane.window, "%zu [  ]  --- %s", i + 1, items.items[i].text);
    mvwvline(todoPane.window, getcury(todoPane.window),
             getcurx(todoPane.window) + PADDING_X, getcury(todoPane.window) + 1,
             0);
    todoPane.totalCells++;
  }

  todoPane.currentCellIndex =
      1; /* You start off on the first index of the cell */

  wrefresh(todoPane.window);
}

/* Initialises the initial, basic user interface */
void initialise_ui(void) {

  initscr();
  clear();
  nonl(); /* Controls where ENTER key is drawn onto page */
  noecho();
  refresh();
  curs_set(0);


  todoPane.window = newwin(LINES, COLS / 2, 0, 0);
  wborder(todoPane.window, 0, 0, 0, 0, 0, 0, 2, 0);

  /* todoWin = newwin(LINES, COLS / 2, 0, 0); */
  /* wborder(todoWin, 0, 0, 0, 0, 0, 0, 2, 0); */
  infoWin = newwin(LINES, COLS / 2, 0, COLS / 2);
  wborder(infoWin, 0, 0, 0, 0, 0, 0, 2, 0);

  /* Colours initialisation */
  start_color();
  init_colour_pairs();

  /* Printing stuff to the top */
  wattron(todoPane.window, A_STANDOUT);
  wattron(todoPane.window, COLOR_PAIR(3));
  wattron(infoWin, A_STANDOUT);
  wattron(infoWin, COLOR_PAIR(3));

  wprintw(todoPane.window, "Enter q/Q to exit.");
  wprintw(infoWin, "Extra information");

  wattroff(todoPane.window, A_STANDOUT);
  wattroff(todoPane.window, COLOR_PAIR(3));
  wattroff(infoWin, COLOR_PAIR(3));
  wattroff(infoWin, A_STANDOUT);

  mvwvline(todoPane.window, PADDING_Y, PADDING_X, 0, 0);

  wrefresh(infoWin);
  wrefresh(todoPane.window);




}

int main(void) {

  initialise_ui();

  init_todo_from_file("example.txt");

  init_display_items_todo_window();

  todo_window_loop();





  system("sleep 1000");

  return 0;
}
