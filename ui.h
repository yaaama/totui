#ifndef UI_H_
#define UI_H_

#include <ncurses.h>

#define PADDING_X 5
#define PADDING_Y 3
#define MAX_TODO_LEN 128
#define MAX_TODO_ITEMS 128


typedef struct Line {
  WINDOW *line; /* Each line will be rendered on a
                    separate window */

  size_t length; /* Length of the line */
  char *head;    /* First char */
  char *tail;    /* Last char (not including null terminator)*/
  char str[MAX_TODO_LEN];

  struct Line *next;     /* Pointers to the next line */
  struct Line *previous; /* Pointer to previous line */

  /* size_t line_number; /\* The number of the line *\/ */
} Line_t;

typedef struct Screen {

  WINDOW *echoBar;              /* Echos information */
  WINDOW *helpBar;    /* Help bar at the top */
  WINDOW **lines;     /* A list of lines (todo entries)
                         (Note this is not a 2d array) */

  size_t lines_total; /* Total number of lines in the screen (number of todo
                         items) */

} Screen_t;

/* Methods */

#endif // UI_H_
