#include "ui.h"
#include <assert.h>
#include <curses.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * Very simple pop-up using ncurses form and menu library (not CDK).
 *
 * The buttons are made from items and the fields are made from... well fields.
 *
 * How to run:
 *	gcc -o test -lmenu -lform -lncurses ncurses-simple-pop-up.c -g && ./test
 */

// Depending on your OS you might need to remove 'ncurses/' from the include
// path.
#include <form.h>
#include <menu.h>

/* External var */
size_t initial_lines_c;

extern Screen_t *scrn;

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

void ui_init_helpbar(void);
void ui_init_line(void);
void ui_init_screen(Line_t **ls);
void ui_refresh(void);

const char *util_get_time(void) {
  time_t rawtime;
  struct tm *timeinfo;
  static char buffer[80];

  time(&rawtime);
  timeinfo = localtime(&rawtime);

  strftime(buffer, 80, "%H:%M:%S", timeinfo);
  return buffer;
}

/* Called on exit
  Will destroy all windows and free mem */
void ui_destroy(void) {

  size_t winc = scrn->lines_total;

  for (size_t i = 0; i < winc; i++) {
    delwin(scrn->lines[i]->window);
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

bool is_todo_ticked(const char *str) {
  const char *ticked_checkbox = "[x]";
  const char *unticked_checkbox = "[ ]";

  // Check if the string contains the ticked checkbox
  if (strstr(str, ticked_checkbox) == str) {
    DEBUG("%s is ticked!", str);
    return true;
  }

  // Check if the string contains the unticked checkbox
  if (strstr(str, unticked_checkbox) == str) {
    DEBUG("%s is not ticked.", str);
    return false;
  }

  DEBUG("%s", "This is neither ticked nor unticked. Error!");
  // TODO Checkbox not found, handle error or default value
  return false;
}

void ui_init_colours(void) {

  start_color();

  init_pair(1, COLOR_RED, COLOR_BLACK);
  init_pair(2, COLOR_GREEN, COLOR_BLACK);
  init_pair(3, COLOR_BLUE, COLOR_BLACK);
  init_pair(4, COLOR_CYAN, COLOR_BLACK);
}

void hl_remove(Line_t *line) {

  wattroff(line->window, A_STANDOUT | A_BOLD);
  wclear(line->window);
  wprintw(line->window, "%s\n", line->item.str);
  wrefresh(line->window);
}

void hl_add(Line_t *line) {

  wattron(line->window, A_STANDOUT | A_BOLD);
  wclear(line->window);
  wprintw(line->window, "%s", line->item.str);
  wrefresh(line->window);
  wattroff(line->window, A_STANDOUT | A_BOLD);
}

void ui_hl_update(Line_t *new, Line_t *old) {

  assert(new &&old != NULL);

  hl_remove(old);

  hl_add(new);
}

void ui_mv_cursor(MOVEMENT_TYPE_t go) {

  Line_t *mvHere = NULL;

  switch (go) {
  case e_mv_up: {
    /* Check if any elements above the current*/
    if (scrn->current_line_index == 0) {
      DEBUG("Cannot move up! Cursor is on the top most element -> %s",
            scrn->currLine->item.str);
      return;
      break;
    }

    mvHere = scrn->currLine->previous;
    scrn->current_line_index--;
    break;
  }
  case e_mv_down: {
    /* Check if any elements below the current */
    if (scrn->current_line_index >= scrn->lines_total - 1) {
      DEBUG("Cannot move down! Cursor on bottom most element -> '%s'",
            scrn->currLine->item.str);
      return;
      break;
    }

    mvHere = scrn->currLine->next;
    scrn->current_line_index++;
    break;
  }
  }

  void *prev = scrn->currLine;
  scrn->currLine = mvHere;

  DEBUG("Cursor now at -> '%s'", scrn->currLine->item.str);
  ui_hl_update(scrn->currLine, prev);
}

void ui_refresh(void) {

  DEBUG("Refreshing all %zu lines!", scrn->lines_total);

  redrawwin(scrn->main);
  wrefresh(scrn->main);
  size_t linec = scrn->lines_total;

  for (size_t i = 0; i < linec; i++) {
    redrawwin(scrn->lines[i]->window);
    wrefresh(scrn->lines[i]->window);
  }
}

void line_append(TodoItem_t item, size_t row) {

  size_t insertionIndex = scrn->lines_total;

  DEBUG("Appending new line with item string: '%s', str length %zu, and item "
        "status %i",
        item.str, item.length, item.status);
  Line_t *line = malloc(sizeof(Line_t));

  line->item = item;
  line->previous = scrn->lines[insertionIndex - 1];
  line->next = NULL;
  scrn->lines[insertionIndex - 1]->next = line;

  scrn->lines[insertionIndex] = line;

  line_render(line, insertionIndex + 1);
  scrn->lines_total++;
}

/* This will print the text on the associated WINDOW of each Line_t and
  also initialise the TodoItem structure. */
void line_render(Line_t *line, size_t row) {

  line->window = newwin(1, COLS - PADDING_X - 1, row, PADDING_X);
  DEBUG("Drawing '%s' onto the screen.", line->item.str);

  bool ticked = is_todo_ticked(line->item.str);

  if (ticked) {
    wattron(line->window, COLOR_PAIR(2));
    line->item.status = e_status_complete;
    wprintw(line->window, "%s", line->item.str);
    wrefresh(line->window);
  } else {
    line->item.status = e_status_incomplete;
    wprintw(line->window, "%s", line->item.str);
    wrefresh(line->window);
  }
}

void ui_init_screen(Line_t **ls) {

  /* TODO Initialise echo bar and help bar */
  scrn->main = newwin(LINES, COLS, 0, 0);
  box(scrn->main, 0, 0);
  wrefresh(scrn->main);
  ui_init_colours();

  scrn->lines = ls;
  scrn->lines_total = 0;
  Line_t *prev = NULL;

  DEBUG("Initialising screen %s", "...");

  /* Iterate through array and initialise each line */
  for (size_t i = 0; i < initial_lines_c; i++) {

    DEBUG("Initialising item '%zu', '%s', length: '%lu'", i,
          scrn->lines[i]->item.str, strlen(scrn->lines[i]->item.str));

    size_t currRow = i + 1;

    /* Creating lines */
    line_render(scrn->lines[i], currRow);

    /* Testing if the string is actually there */
    assert(scrn->lines[i]->item.str != NULL);

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
        scrn->lines_total, scrn->currLine->item.str, scrn->current_line_index);
}

Screen_t *ui_init(Line_t **lines) {

  initscr();
  cbreak();
  /* nonl(); /\* Controls where ENTER key is drawn onto page *\/ */
  noecho();
  raw();
  curs_set(0);
  keypad(stdscr, true);
  box(stdscr, 0, 0);

  scrn = malloc(sizeof(Screen_t));
  ui_init_screen(lines);

  return scrn;
}
