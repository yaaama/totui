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
void ui_init_screen(LineList_t *ls);
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

/* Free the entire list */
/* Called on exit
  Will destroy all windows and free mem */
void ui_destroy(void) {

  Line_t *curr = scrn->lines->head;

  while (curr) {
    delwin(curr->window);
    Line_t *next = curr->next;
    free(curr);
    curr = next;
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
            scrn->lines->current_line->item.str);
      return;
      break;
    }

    mvHere = scrn->lines->current_line->previous;
    scrn->current_line_index--;
    break;
  }
  case e_mv_down: {
    /* Check if any elements below the current */
    if (scrn->current_line_index >= scrn->lines->size - 1) {
      DEBUG("Cannot move down! Cursor on bottom most element -> '%s'",
            scrn->lines->current_line->item.str);
      return;
      break;
    }

    mvHere = scrn->lines->current_line->next;
    scrn->current_line_index++;
    break;
  }
  }

  Line_t *prev = scrn->lines->current_line;
  scrn->lines->current_line = mvHere;

  DEBUG("Cursor now at -> '%s'", scrn->lines->current_line->item.str);
  ui_hl_update(scrn->lines->current_line, prev);
}

void ui_refresh(void) {

  DEBUG("Refreshing all %zu lines!", scrn->lines->size);

  redrawwin(scrn->main);
  wrefresh(scrn->main);

  Line_t *curr = scrn->lines->head;
  while (curr) {
    redrawwin(curr->window);
    wrefresh(curr->window);
    curr = curr->next;
  }
}

void line_remove_current(void) {

  Line_t *curr = scrn->lines->current_line;

  Line_t *newCurr = curr->next;
  newCurr->previous = curr->previous;
  newCurr->previous->previous = curr->next;

  scrn->lines->current_line = newCurr;

  scrn->lines->size--;

  /* Free mem taken by curr */
  delwin(curr->window);
  free(curr);

  ui_refresh();
}

void line_append(TodoItem_t item) {

  LineList_t *list = scrn->lines;
  Line_t *newLine = malloc(sizeof(Line_t));
  if (!newLine)
    return;

  newLine->item = item;
  newLine->next = NULL;
  newLine->previous = list->tail;

  if (list->tail) {
    list->tail->next = newLine;
  } else {
    list->head = newLine;
  }

  list->tail = newLine;
  list->size++;
  line_render(newLine, list->size);
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

void init_lines_linked_list(LineList_t *list) {

  assert(list != NULL && "This list of lines is NULL");

  Line_t *curr = list->head;
  size_t i = 0;
  while (curr) {
    DEBUG("Initialising item '%zu', '%s', length: '%lu'", i, curr->item.str,
          curr->item.length);
    line_render(curr, i + 1);
    curr = curr->next;
    ++i;
  }
  list->current_line = list->head;

  scrn->lines = list;
}

void ui_init_screen(LineList_t *ls) {

  DEBUG("Initialising screen %s", "...");
  /* TODO Initialise echo bar and help bar */
  scrn->main = newwin(LINES, COLS, 0, 0);
  box(scrn->main, 0, 0);
  wrefresh(scrn->main);
  ui_init_colours();

  /* Set up lines linked list */
  init_lines_linked_list(ls);
  hl_add(scrn->lines->current_line);
}

Screen_t *ui_init(LineList_t *lines) {

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
