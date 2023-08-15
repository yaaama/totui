/*********************/
/* /\* Libraries *\/ */
/*********************/

/* My header files */
#include "ui.h"
/* Standard C libs */
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
/* Curse libs */
#include <curses.h>
#include <form.h>
#include <menu.h>
#include <ncurses.h>

/**************************/
/* /\* Global Variables *\/ */
/**************************/

/* Number of todo items the program initially loads up */
size_t initial_lines_c;
/* Where all of the UI information + todos are stored */
extern Screen_t *scrn;

/***************/
/* Functions */
/***************/
void ui_init_helpbar(void);
void ui_init_line(void);
void init_main_screen(void);
void ui_refresh(void);

/* Returns local time, used for debugging
 TODO Move this into a new file */
const char *util_get_time(void) {
  time_t rawtime;
  struct tm *timeinfo;
  static char buffer[80];
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  strftime(buffer, 80, "%H:%M:%S", timeinfo);
  return buffer;
}

/* Frees mem from the entire list and destroys the UI.
 * Called on exit */
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

/* Checks for whether a todo item string is ticked or not. */
bool is_todo_ticked(const char *str) {
  const char *ticked_checkbox = "[x]";
  const char *unticked_checkbox = "[ ]";

  // Check if the string contains a ticked checkbox
  if (strstr(str, ticked_checkbox) == str) {
    DEBUG("%s is ticked!", str);
    return true;
  }

  // Check if the string contains a unticked checkbox
  if (strstr(str, unticked_checkbox) == str) {
    DEBUG("%s is not ticked.", str);
    return false;
  }

  DEBUG("%s", "This is neither ticked nor unticked. Error!");
  assert(false && "TODO Checkbox not found. Handle this.");
  return false;
}

/* Will set the colours for ncurses to use
 * TODO I should probably customise these when I have the time. */
void ui_init_colours(void) {

  start_color();

  init_pair(1, COLOR_RED, COLOR_BLACK);
  init_pair(2, COLOR_GREEN, COLOR_BLACK);
  init_pair(3, COLOR_BLUE, COLOR_BLACK);
  init_pair(4, COLOR_CYAN, COLOR_BLACK);
}

/* Removes highlighting from line. */
void hl_remove(Line_t *line) {
  // TODO Turn the default highlighting into a macro
  wattroff(line->window, A_STANDOUT | A_BOLD);
  wclear(line->window);
  wprintw(line->window, "%s\n", line->item.str);
  wrefresh(line->window);
}
/* Adds highlighting to a line. */
void hl_add(Line_t *line) {
  wattron(line->window, A_STANDOUT | A_BOLD);
  wclear(line->window);
  wprintw(line->window, "%s", line->item.str);
  wrefresh(line->window);
  wattroff(line->window, A_STANDOUT | A_BOLD);
}

/* Will update the highlighting by removing the highlight from an old
   line and then highlighting a new line.
 * This is necessary for when the user moves the cursor up and down.
 * The highlighting displays to the user which line they are on. */
void ui_hl_update(Line_t *new, Line_t *old) {

  assert(new != NULL);

  if (old) {
    hl_remove(old);
  }

  hl_add(new);
}

/* TODO Will display a popup for the user when the window is empty */
void ui_empty_todolist(void) {
  assert(false && "Implement a popup to tell the user that the menu is empty.");

  /* box(scrn->main, LINES / 4, COLS / 4); */
  /* wattron(scrn->main, A_BOLD | A_BLINK | A_STANDOUT | A_UNDERLINE); */
  /* wprintw(scrn->main, */
  /*         "There are no items in your todo list! Press 'a' to add a new
   * one."); */
  /* wattroff(scrn->main, A_BOLD | A_BLINK | A_STANDOUT | A_UNDERLINE); */

  box(scrn->main, LINES / 4, COLS / 4);
}

/* This function moves the cursor either up or down. */
void ui_mv_cursor(MOVEMENT_TYPE_t go) {

  /* Cant move a cursor if there are no lines. */
  if (scrn->lines->size == 0) {
    DEBUG("--> Screen empty, movement is impossible.");
  }

  Line_t *mvHere = NULL;
  Line_t *currLine = scrn->lines->current_line;

  /* If the current line is null for whatever reason, then the current line
   * should be replaced with the first line. */
  if (currLine == NULL) {
    scrn->lines->current_line = scrn->lines->head;
  }

  /* Switch case that evaluates the MOVEMENT_TYPE_t enum: */
  switch (go) {

    /* Cursor is being moved up! */
  case e_mv_up: {
    /* Check if any elements above the current: */
    if (!currLine->previous) {
      DEBUG("Cannot move up! Cursor is on the top most element -> %s",
            scrn->lines->current_line->item.str);
      return;
    }
    /* When you want to move up, that translates to going to the previous item
     * in our linked list and vice versa */
    mvHere = currLine->previous;
    scrn->current_line_index--;
    break;
  }

    /* Cursor is being moved down... */
  case e_mv_down: {
    /* Check if any elements below the current */
    if (!currLine->next) {
      DEBUG("Cannot move down! Cursor on bottom most element -> '%s'",
            scrn->lines->current_line->item.str);
      return;
    }
    mvHere = currLine->next;
    scrn->current_line_index++;
    break;
  }
  }

  scrn->lines->current_line = mvHere;

  DEBUG("Cursor moved! Now at -> '%s'", scrn->lines->current_line->item.str);
  ui_hl_update(scrn->lines->current_line,
               currLine); /* Updating the highlighting */
}

/* ui_refresh refreshes the screen. */
void ui_refresh(void) {

  if (scrn->lines->size == 0) { /* Why bother refreshing an empty screen? */
    DEBUG("No lines to refresh, returning...");
    return;
  }

  /* If todo list is just 1 item, then hl that 1 item.. */
  if (scrn->lines->size == 1) {
    hl_add(scrn->lines->current_line);
  }

  DEBUG("Refreshing all %zu lines!", scrn->lines->size);

  /* Redraw the main screen so it doesn't contain any weird artifacts */
  redrawwin(scrn->main);
  wrefresh(scrn->main);

  /* Start refreshing from the start (the head) of the list*/
  Line_t *curr = scrn->lines->head;

  while (curr) {
    DEBUG("Attempting to redraw item: %s", curr->item.str);
    redrawwin(curr->window);
    wrefresh(curr->window);
    curr = curr->next;
  }
}

/* Use this function after deleting an item.
 * Pass in the Y coordinate of the item being deleted so the items below it can
   be shifted upwards. */
void ui_refresh_delete(size_t delWinY) {

  DEBUG("--> Refreshing (after deletion) %zu lines!", scrn->lines->size);

  redrawwin(scrn->main);
  wrefresh(scrn->main);
  size_t newY = delWinY;

  Line_t *curr = scrn->lines->head;
  size_t currY = curr->window->_begy;

  while (curr) {
    DEBUG("Attempting to redraw item: %s", curr->item.str);
    if (currY >= delWinY) {

      line_render(curr, newY);

      redrawwin(curr->window);
      wrefresh(curr->window);
      curr = curr->next;
      newY++;
      continue;
    }

    redrawwin(curr->window);
    wrefresh(curr->window);
    curr = curr->next;
    if (curr)
      currY = curr->window
                  ->_begy; /* If curr is NON NULL then it will reassign the Y */
  }
}

/* Append a new Line_t to the scrn->lines linked list.
 * This function should be self explanatory. Give an TodoItem_t and it will
 * malloc a new Line_t and add it to the linked list. Note that the function
 * does not render a new window for it. */
void line_append(TodoItem_t item) {

  DEBUG("Appending new item: %s", item.str);
  LineList_t *list = scrn->lines;
  Line_t *newLine = malloc(sizeof(Line_t));
  if (!newLine) {
    DEBUG("Malloc failed...");
    return;
  }
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

  if (list->size == 1) {
    list->current_line = list->head;
  }
}

/* This will print the todo item text stored in TodoItem_t.str on the associated
WINDOW of each Line_t. * Also colours line based on completion status.
 * Used to render a line onto the screen when a new item is created.
*/
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

/* Called by ui_init_screen
 * Takes in a LineList_t and renders all of the Lines. */
void render_all_lines(LineList_t *list) {

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

/* Draws up the main window stored in scrn->main */
void init_main_screen(void) {

  DEBUG("Initialising screen %s", "...");
  /* TODO Initialise echo bar and help bar */
  scrn->main = newwin(LINES, COLS, 0, 0);
  box(scrn->main, 0, 0);
  wrefresh(scrn->main);
  ui_init_colours();
}

/* Called when the program starts
 * Initialises the screen and sets some ncurses options whilst also passing the
 * LineList to get drawn  */
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
  init_main_screen();

  /* Set up lines linked list */
  render_all_lines(lines);
  hl_add(scrn->lines->current_line);

  return scrn;
}
