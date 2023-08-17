#ifndef UI_H_
#define UI_H_

/* Standard C libs */
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
/* Curse libs */
#include <curses.h>
#include <dialog.h>
#include <form.h>
#include <menu.h>
#include <ncurses.h>

#define PADDING_X 5
#define PADDING_Y 3
#define MAX_TODO_LEN 128
#define MAX_TODO_ITEMS 128

#define ATTR_DONE COLOR_PAIR(2)
#define ATTR_TODO COLOR_PAIR(1)
#define ATTR_CURR_LINE A_BOLD | A_STANDOUT

/****************************/
/* /\* Type definitions *\/ */
/****************************/

/* Used to describe what action is being taken
  NOTE: Not yet implemented */
typedef enum ACTION_TYPE {
  e_cmd_add_item,
  e_cmd_remove_item,
  e_cmd_toggle_item,
  e_cmd_move_cursor
} ACTION_TYPE_t;

/* Type of movement of the cursor */
typedef enum MOVEMENT_TYPE { e_mv_down, e_mv_up } MOVEMENT_TYPE_t;

/* Todo status */
typedef enum TODO_STATUS {
  e_status_complete,
  e_status_incomplete
  /* NOTE: Add more statuses later down the line. */
} TODO_STATUS_t;

/* TODO Dimensions, not yet used */
typedef struct Dimensions {
  size_t x;
  size_t y;
} Dimensions_t;

/* Structure of a todo item */
typedef struct TodoItem {

  size_t length;          /* Length of the line */
  char str[MAX_TODO_LEN]; /* Todo contents */
  TODO_STATUS_t status;   /* Status of the todo item */

} TodoItem_t;

/* A Line is a node stored in the LineList type.
 * It contains a WINDOW type that is used to display the TodoItem it contains on
the screen. */
typedef struct Line {
  WINDOW *window; /* Each line will be rendered on a separate window */
  struct Line *next;
  struct Line *previous;
  struct TodoItem item;
} Line_t;

/* A doubly linked list of Line's */
typedef struct LineList {
  struct Line *head;
  struct Line *tail;
  struct Line *current_line;
  size_t size;

} LineList_t;

/* Structure that contains the entire screen contents. */
typedef struct Screen {
  WINDOW *main;
  WINDOW *echo_bar; /* Echos information */
  WINDOW *help_bar; /* Help bar at the top */
  LineList_t *lines;
  /* size_t lines_total; /\* Total number of lines in the screen *\/ */
  /* Line_t *currLine; */
  size_t current_line_index;
} Screen_t;

/**************************/
/* /\* Methods for UI *\/ */
/**************************/
Screen_t *ui_init(LineList_t *ls);
void linelist_destroy(LineList_t *list);
void ui_hl_update(Line_t *new, Line_t *old);
void ui_destroy(void);
void ui_mv_cursor(MOVEMENT_TYPE_t go);
int createForm(void);
void ui_refresh(void);
void line_render(Line_t *line, size_t row);
void line_list_add_new_item(TodoItem_t *item);
void ui_remove_line(void);
void ui_refresh_delete(size_t delWinY);
void ui_empty_todolist(void);

/***************************/
/* /\* Utility methods *\/ */
/***************************/
bool scrn_lines_empty(void);

#endif // UI_H_
