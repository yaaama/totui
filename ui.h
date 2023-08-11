#ifndef UI_H_
#define UI_H_

#include <ncurses.h>
#include <time.h>

#define LOG_FILE "log.txt"

#define DEBUG(fmt, ...)                                                        \
  do {                                                                         \
    FILE *file = fopen(LOG_FILE, "a");                                         \
    if (file != NULL) {                                                        \
      fprintf(file, "%s\t%s: " fmt "\n", util_get_time(), __func__,            \
              ##__VA_ARGS__);                                                  \
      fclose(file);                                                            \
    }                                                                          \
  } while (0)

#define PADDING_X 5
#define PADDING_Y 3
#define MAX_TODO_LEN 128
#define MAX_TODO_ITEMS 128

/****************************/
/* /\* Type definitions *\/ */
/****************************/
typedef enum ACTION_TYPE {
  e_cmd_add_item,
  e_cmd_remove_item,
  e_cmd_toggle_item,
  e_cmd_move_cursor
} ACTION_TYPE_t;
typedef enum MOVEMENT_TYPE { e_mv_down, e_mv_up } MOVEMENT_TYPE_t;
typedef enum TODO_STATUS {
  e_status_complete,
  e_status_incomplete
} TODO_STATUS_t;

typedef struct Dimensions {
  size_t x;
  size_t y;
} Dimensions_t;

typedef struct TodoItem {

  size_t length; /* Length of the line */
  char str[MAX_TODO_LEN];
  TODO_STATUS_t status;

} TodoItem_t;

typedef struct Line {
  WINDOW *window; /* Each line will be rendered on a separate window */
  struct Line *next;
  struct Line *previous;
  struct TodoItem item;
} Line_t;

typedef struct LineList {

  struct Line *head;
  struct Line *tail;
  size_t size;

} LineList_t;

typedef struct Screen {

  WINDOW *main;
  WINDOW *echo_bar; /* Echos information */
  WINDOW *help_bar; /* Help bar at the top */
  /* Line_t **lines;     /\* A list of lines (todo entries) *\/ */
  LineList_t *lines;
  /* size_t lines_total; /\* Total number of lines in the screen *\/ */
  Line_t *currLine;
  size_t current_line_index;
} Screen_t;

/****************************/
/* /\* Global variables *\/ */
/****************************/

// file name global variable
extern char todo_file_name[64];
// Lines initially loaded from file
extern size_t initial_lines_c;

/**************************/
/* /\* Methods for UI *\/ */
/**************************/
Screen_t *ui_init(Line_t **lines);
void ui_hl_update(Line_t *new, Line_t *old);
/* void ui_mv_up(Screen_t *scrn); */
/* void ui_mv_down(Screen_t *scrn); */
void ui_destroy(void);
void ui_mv_cursor(MOVEMENT_TYPE_t go);
int createForm(void);
void ui_refresh(void);
void line_render(Line_t *line, size_t row);
void line_append(TodoItem_t item, size_t row);

/***************************/
/* /\* Utility methods *\/ */
/***************************/
const char *util_get_time(void);

#endif // UI_H_
