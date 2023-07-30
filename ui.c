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

/* Called on exit
  Will destroy all windows and free mem */
void ui_destroy(Screen_t *scrn) {

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

void ui_mv_down(Screen_t *scrn) {

  Line_t *new = NULL;
  DEBUG("--> Attempting to go down.");
  if (scrn->current_line_index >= scrn->lines_total - 1) {
    DEBUG("Cannot move down! Trying to go to index %zu, but only %zu elements.",
          scrn->current_line_index + 1, scrn->lines_total);
    /* TODO Print error */
    return;
  }
  DEBUG("Current line index: %zu, requesting to go DOWN, '%s'",
        scrn->current_line_index, scrn->currLine->next->item.str);

  new = scrn->currLine->next;
  scrn->current_line_index++;

  void *prev = scrn->currLine;
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
        scrn->current_line_index, scrn->currLine->previous->item.str);

  new = scrn->currLine->previous;
  scrn->current_line_index--;

  void *prev = scrn->currLine;
  scrn->currLine = new;
  ui_hl_update(scrn->currLine, prev);
}

/* This will print the text on the associated WINDOW of each Line_t and
  also initialise the TodoItem structure. */
void create_line(Line_t *line, size_t row) {

  line->window = newwin(1, COLS - PADDING_X - 1, row, PADDING_X);
  DEBUG("Drawing '%s' onto the screen.", line->item.str);

  bool ticked = is_todo_ticked(line->item.str);

  if (ticked) {
    wattron(line->window, COLOR_PAIR(2));
    line->item.status = S_COMPLETION;
    wprintw(line->window, "%s", line->item.str);
    wrefresh(line->window);
  } else {
    line->item.status = S_INCOMPLETE;
    wprintw(line->window, "%s", line->item.str);
    wrefresh(line->window);
  }
}

void ui_init_screen(Screen_t *scrn, Line_t **ls) {

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
    create_line(scrn->lines[i], currRow);

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

  Screen_t *scrn = malloc(sizeof(Screen_t));
  ui_init_screen(scrn, lines);

  return scrn;
}
