#include "ui.h"
#include <assert.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* External var */
size_t initial_lines_c;

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

  DEBUG("Drawing '%s' onto the screen... ", line->str);
  wprintw(line->ui_line, "%s", line->str);
  wrefresh(line->ui_line);
}

void ui_init_screen(Screen_t *scrn , Line_t **ls) {

  /* TODO Initialise echo bar and help bar */

  scrn->lines = ls;
  scrn->lines_total = 0;


  /* Iterate through array and initialise each line */
  for (size_t i = 0; i < initial_lines_c; i++) {

    DEBUG("Init started for item numb %zu, '%s'..., length: %lu", i, scrn->lines[i]->str, strlen(scrn->lines[i]->str));
    size_t currRow = i + 1;
    scrn->lines[i]->ui_line = newwin(1, COLS - PADDING_X, currRow, PADDING_X - 1);
    ui_init_lines(scrn->lines[i]);

    /* Testing if the string is actually there */
    assert(scrn->lines[i]->str != NULL);

    scrn->lines_total++;
  }

  DEBUG("Total lines initialised: %zu", scrn->lines_total);

}

Screen_t *ui_init(Line_t **lines) {

  initscr();
  cbreak();
  /* nonl(); /\* Controls where ENTER key is drawn onto page *\/ */
  noecho();

  box(stdscr, 0, 0);
  /* mvcur(0, 0, LINES + 20, 0); */

  /* mvcur(10, 10, 10, 30); */
  /* printw("whats up dude"); */
  mvprintw(2, 30, "Whats up dude\n");
  wrefresh(stdscr);
  /* curs_set(0); */

  Screen_t *scrn = malloc(sizeof(Screen_t));
 ui_init_screen(scrn, lines);

 return scrn;
}
