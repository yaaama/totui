#ifndef UI_H_
#define UI_H_

#include <ncurses.h>
#include <time.h>

#define LOG_FILE "log.txt"

#define DEBUG(fmt, ...) do { \
    FILE *file = fopen(LOG_FILE, "a"); \
    if (file != NULL) { \
        fprintf(file, "%s\t%s: " fmt "\n", get_time(), __func__, ##__VA_ARGS__); \
        fclose(file); \
    } \
} while (0)


#define PADDING_X 5
#define PADDING_Y 3
#define MAX_TODO_LEN 128
#define MAX_TODO_ITEMS 128

const char* get_time(void);

typedef struct Line {
  WINDOW *ui_line; /* Each line will be rendered on a
                    separate window */
  size_t length; /* Length of the line */
  char str[MAX_TODO_LEN];
} Line_t;

typedef struct Screen {

  WINDOW *main;
  WINDOW *echo_bar;              /* Echos information */
  WINDOW *help_bar;    /* Help bar at the top */
  Line_t **lines;     /* A list of lines (todo entries) */
  size_t lines_total; /* Total number of lines in the screen (number of todo
                         items) */
} Screen_t;

/* Methods */
void ui_init(Line_t **lines);

#endif // UI_H_
