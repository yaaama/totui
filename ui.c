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
void ui_init_helpbar(Screen_t *scrn);
void ui_init_line(Screen_t *scrn);

const char *util_get_time(void) {
  time_t rawtime;
  struct tm *timeinfo;
  static char buffer[80];

  time(&rawtime);
  timeinfo = localtime(&rawtime);

  strftime(buffer, 80, "%H:%M:%S", timeinfo);
  return buffer;
}

void ui_destroy(Screen_t *scrn) {

  size_t winc = scrn->lines_total;
  for (size_t i = 0 ; i < winc; i ++) {
    WINDOW *currWin = scrn->lines[i]->ui_line;
    delwin(currWin);
    free(scrn->lines[i]);
  }

  delwin(scrn->main);
  delwin(scrn->echo_bar);
  delwin(scrn->help_bar);

  free(scrn->lines);
  free(scrn);

  endwin();
  refresh();
  endwin();

}




void ui_init_colours(void) {

  start_color();

  init_pair(1, COLOR_RED, COLOR_RED);
  init_pair(2, COLOR_GREEN, COLOR_BLACK);
  init_pair(3, COLOR_BLUE, COLOR_BLACK);
  init_pair(4, COLOR_CYAN, COLOR_BLACK);
}

void hl_remove(Line_t *line) {

  wattroff(line->ui_line, A_STANDOUT | A_BOLD);
  wclear(line->ui_line);
  wprintw(line->ui_line, "%s\n", line->str);
  wrefresh(line->ui_line);
}

void hl_add(Line_t *line) {

  wattron(line->ui_line, A_STANDOUT | A_BOLD);
  /* wattron(line->ui_line, A_BOLD); */
  wclear(line->ui_line);
  wprintw(line->ui_line, "%s", line->str);
  wattroff(line->ui_line, A_STANDOUT | A_BOLD);
  /* wattroff(line->ui_line, A_BOLD); */

  wrefresh(line->ui_line);
}

void ui_hl_update(Line_t *new, Line_t *old) {

  assert(new &&old != NULL);

  hl_remove(old);
  hl_add(new);
}

void ui_mv_down(Screen_t *scrn) {

  Line_t *new = NULL;
    DEBUG("--> Attempting to go down.");
    if (scrn->current_line_index >= scrn->lines_total - 1) {
      DEBUG(
          "Cannot move down! Trying to go to index %zu, but only %zu elements.",
          scrn->current_line_index + 1, scrn->lines_total);
      /* TODO Print error */
      return;
    }
    DEBUG("Current line index: %zu, requesting to go DOWN, '%s'",
          scrn->current_line_index, scrn->currLine->next->str);

    new = scrn->currLine->next;
    scrn->current_line_index++;

  void* prev = scrn->currLine;
  scrn->currLine = new;
  ui_hl_update(scrn->currLine, prev);
}

void ui_mv_up(Screen_t *scrn) {

  Line_t *new = NULL;

  DEBUG("--> Attempting to go up.");
  if (scrn->current_line_index == 0) {
    DEBUG("Cannot move up! This is the first element.");
    /* TODO Print error here */
    return;
  }

  DEBUG("Current line index: %zu, requesting to go UP, '%s'",
        scrn->current_line_index, scrn->currLine->previous->str);

  new = scrn->currLine->previous;
  scrn->current_line_index--;

  void* prev = scrn->currLine;
  scrn->currLine = new;
  ui_hl_update(scrn->currLine, prev);
}

/* This will print the text on the associated WINDOW of each Line_t */
void create_line(Line_t *line, size_t row) {

  line->ui_line = newwin(1, COLS - PADDING_X - 1, row, PADDING_X);

  DEBUG("Drawing '%s' onto the screen.", line->str);
  wprintw(line->ui_line, "%s", line->str);
  wrefresh(line->ui_line);
}

void ui_init_screen(Screen_t *scrn, Line_t **ls) {

  /* TODO Initialise echo bar and help bar */
  scrn->main = newwin(LINES, COLS, 0, 0);
  box(scrn->main, 0, 0);
  wrefresh(scrn->main);

  scrn->lines = ls;
  scrn->lines_total = 0;
  Line_t *prev = NULL;

  DEBUG("Initialising screen %s", "...");

  /* Iterate through array and initialise each line */
  for (size_t i = 0; i < initial_lines_c; i++) {

    DEBUG("Initialising item '%zu', '%s', length: '%lu'", i,
          scrn->lines[i]->str, strlen(scrn->lines[i]->str));

    size_t currRow = i + 1;

                                /* Creating lines */
    create_line(scrn->lines[i], currRow);

    /* Testing if the string is actually there */
    assert(scrn->lines[i]->str != NULL);

    scrn->lines_total++;

    /* Linking nodes together (doubly linked list) */
    scrn->lines[i]->next = NULL;
    scrn->lines[i]->previous = prev;
    if (prev != NULL) {
      prev->next = scrn->lines[i];
    }
    prev = scrn->lines[i];
  }

  scrn->currLine = scrn->lines[scrn->lines_total - 1];
  scrn->current_line_index = scrn->lines_total - 1;
  hl_add(scrn->currLine);

  DEBUG("---> Initialised %zu lines, current line is '%s', at index '%zu'.",
        scrn->lines_total, scrn->currLine->str, scrn->current_line_index);
}

Screen_t *ui_init(Line_t **lines) {

  initscr();
  cbreak();
  /* nonl(); /\* Controls where ENTER key is drawn onto page *\/ */
  noecho();
  raw();
  curs_set(0);

  box(stdscr, 0, 0);
  /* mvcur(0, 0, LINES + 20, 0); */

  /* mvcur(10, 10, 10, 30); */
  /* printw("whats up dude"); */

  Screen_t *scrn = malloc(sizeof(Screen_t));
  ui_init_screen(scrn, lines);

  return scrn;
}
