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
typedef enum KEY_EVENT { k, j, CREATE, DELETE } KEY_EVENT_t;
typedef enum MOVEMENT_TYPE { UP, DOWN } MOVEMENT_TYPE_t;

typedef struct Dimensions {
  size_t x;
  size_t y;
} Dimensions_t;


typedef struct Line {
  WINDOW *ui_line; /* Each line will be rendered on a separate window */
  size_t length;   /* Length of the line */
  char str[MAX_TODO_LEN];
  struct Line *next;
  struct Line *previous;
} Line_t;

typedef struct Screen {

  WINDOW *main;
  WINDOW *echo_bar;   /* Echos information */
  WINDOW *help_bar;   /* Help bar at the top */
  Line_t **lines;     /* A list of lines (todo entries) */
  size_t lines_total; /* Total number of lines in the screen */
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
void ui_mv_up(Screen_t *scrn);
void ui_mv_down(Screen_t *scrn);

/***************************/
/* /\* Utility methods *\/ */
/***************************/
const char *util_get_time(void);
#endif // UI_H_
