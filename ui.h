#ifndef UI_H_
#define UI_H_

#include <ncurses.h>
#include <time.h>

#define LOG_FILE "log.txt"

#define DEBUG(fmt, ...)                                                        \
  do {                                                                         \
    FILE *file = fopen(LOG_FILE, "a");                                         \
    if (file != NULL) {                                                        \
      fprintf(file, "%s\t%s: " fmt "\n", get_time(), __func__, ##__VA_ARGS__); \
      fclose(file);                                                            \
    }                                                                          \
  } while (0)

#define PADDING_X 5
#define PADDING_Y 3
#define MAX_TODO_LEN 128
#define MAX_TODO_ITEMS 128

// Stores the number of items loaded up from file
extern size_t initial_lines_c;
// Todo file global variable
extern char todo_file_name[64];

const char *get_time(void);

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
  struct Line *currLine;
  size_t current_line_index;
} Screen_t;

/* Methods */
Screen_t *ui_init(Line_t **lines);

#endif // UI_H_
