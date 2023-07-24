#include "ui.h"
#include <assert.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* External var */
size_t initial_file_lines_count;

/***************/
/* Definitions */
/***************/
/* void ui_init_attribute(void); */
/* void ui_init_colors(void); */
/* void ui_highlight_line(Line_t *line); */
/* void ui_refresh_screen(void); */

/**************************/
/* /\* Initialisation *\/ */
/**************************/
/* void ui_init_helpbar(void); */
/* void ui_init_line(void); */

const char *get_time(void) {
  time_t rawtime;
  struct tm *timeinfo;
  static char buffer[80];

  time(&rawtime);
  timeinfo = localtime(&rawtime);

  strftime(buffer, 80, "%H:%M:%S", timeinfo);
  return buffer;
}

void ui_init_colours(void) {

  start_color();

  init_pair(1, COLOR_RED, COLOR_RED);
  init_pair(2, COLOR_GREEN, COLOR_BLACK);
  init_pair(3, COLOR_BLUE, COLOR_BLACK);
  init_pair(4, COLOR_CYAN, COLOR_BLACK);
}

/* This will print the text on the associated WINDOW of each Line_t */
void ui_init_lines(Line_t *line) {

  DEBUG("Received '%s' ", line->str);
  wprintw(line->ui_line, "%s", line->str);
  wrefresh(line->ui_line);
}

Screen_t *ui_init_screen(Line_t **ls) {

  Screen_t *screen = malloc(sizeof(Screen_t));
  screen->lines = ls;
  screen->lines_total = 0;

  /* TODO Initialise echo bar and help bar */

  /* Initialising each line from the lines array and then adding them to the
   * screen struct  */
  for (size_t i = 0; i < initial_file_lines_count; i++) {

    DEBUG("Currently reading %s...", screen->lines[i]->str);
    size_t currRow = i + 1;
    screen->lines[i]->ui_line = newwin(1, COLS, currRow, PADDING_X);
    ui_init_lines(
        screen->lines[i]); /* Increments lines after calling ui_init_lines */

    assert(screen->lines[i]->str !=
           NULL); /* Testing if the string is actually there */

    screen->lines_total++;
  }

  return screen;
}

void ui_init(Line_t **lines) {

  initscr();
  cbreak();
  /* nonl(); /\* Controls where ENTER key is drawn onto page *\/ */
  noecho();
  refresh();

  box(stdscr, 0, 0);
  /* mvcur(0, 0, LINES + 20, 0); */

  /* mvcur(10, 10, 10, 30); */
  /* printw("whats up dude"); */
  mvprintw(2, 30, "Whats up dude\n");
  wrefresh(stdscr);
  /* curs_set(0); */

  Screen_t *scrn = ui_init_screen(lines);

  getch();
}
