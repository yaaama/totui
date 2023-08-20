/*********************/
/* /\* Libraries *\/ */
/*********************/

/* My header files */
#include "ui.h"
#include "util.h"

/**************************/
/* /\* Global Variables *\/ */
/**************************/

/* Where all of the UI information + todos are stored */
extern Screen_t *scrn;

/***************/
/* Functions */
/***************/
void ui_init_helpbar(void); /* TODO */
void ui_init_line(void);    /* TODO */
void init_main_screen(void);
void ui_refresh(void);
void line_wprint(Line_t *line);

/* Frees mem from the entire list and destroys the UI.
 * Called on exit */
void ui_destroy(void) {

  Line_t *curr = scrn->lines->head;

  while (curr) {
    delwin(curr->window);
    Line_t *next = curr->next;
    free(curr->item);
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

/* Will set the colours for ncurses to use
 * TODO I should probably customise these when I have the time. */
void ui_init_colours(void) {

  start_color();

  init_pair(1, COLOR_RED, COLOR_BLACK);
  init_pair(2, COLOR_GREEN, COLOR_BLACK);
  init_pair(3, COLOR_BLUE, COLOR_BLACK);
  init_pair(4, COLOR_CYAN, COLOR_BLACK);
}

/* Clears the window and then prints to the window in the line struct*/
void line_wprint(Line_t *line) {

  assert(line != NULL && "Line is NULL.");
  char *statStr = convert_status_to_box(line->item->status);
  wclear(line->window);
  wprintw(line->window, "%s %s", statStr, line->item->str);
}

/* Will update the highlighting by removing the highlight from an old
   line and then highlighting a new line.
 * This is necessary for when the user moves the cursor up and down.
 * The highlighting displays to the user which line they are on. */
void ui_hl_update(Line_t *new, Line_t *old) {

  /* If there is an old line to remove highlighting from... */
  if (old) {
    wattroff(old->window, ATTR_CURR_LINE);
    wclear(old->window);
    line_wprint(old);
    wrefresh(old->window);
  }

  wclear(new->window);
  wattron(new->window, ATTR_CURR_LINE);
  line_wprint(new);
  wrefresh(new->window);
  wattroff(new->window, ATTR_CURR_LINE);
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
void ui_mv_cursor(MOVEMENT_TYPE_e go) {

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
            scrn->lines->current_line->item->str);
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
            scrn->lines->current_line->item->str);
      return;
    }
    mvHere = currLine->next;
    scrn->current_line_index++;
    break;
  }
  }

  scrn->lines->current_line = mvHere;

  DEBUG("Cursor moved! Now at -> '%s'", scrn->lines->current_line->item->str);
  ui_hl_update(scrn->lines->current_line,
               currLine); /* Updating the highlighting */
}

void ui_resized(void) {

  resizeterm(LINES, COLS);
  wresize(scrn->main, LINES, COLS);
  box(scrn->main, LINES, COLS);
  wrefresh(scrn->main);
}

/* ui_refresh refreshes the screen. */
void ui_refresh(void) {

  // TODO Add resize handling here.
  /* if (is_term_resized(int, int)) { */
  /*   ui_resized(); */
  /* } */

  if (scrn->lines->size == 0) { /* Why bother refreshing an empty screen? */
    DEBUG("No lines to refresh, returning...");
    return;
  }

  /* If todo list is just 1 item then hl that 1 item. */
  if (scrn->lines->size == 1) {
    ui_hl_update(scrn->lines->current_line, NULL);
  }

  DEBUG("Refreshing all %zu lines!", scrn->lines->size);

  /* Redraw the main screen so it doesn't contain any weird artifacts */
  redrawwin(scrn->main);
  wrefresh(scrn->main);

  /* Start refreshing from the start (the head) of the list*/
  Line_t *curr = scrn->lines->head;

  while (curr) {
    redrawwin(curr->window);
    wrefresh(curr->window);
    curr = curr->next;
  }
  DEBUG("%s", "UI Refreshed.");
}

/* Use this function after deleting an item
 * Pass in the Y coordinate of the item being deleted so the item below it
 can be shifted upwards. */
void ui_refresh_delete(size_t delWinY) {

  DEBUG("--> Refreshing (after deletion) %zu lines!", scrn->lines->size);

  redrawwin(scrn->main);
  wrefresh(scrn->main);
  size_t newY = delWinY;

  Line_t *curr = scrn->lines->head;
  size_t currY = curr->window->_begy;

  while (curr) {
    DEBUG("Attempting to redraw item-> %s", curr->item->str);
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
 * This function should be self explanatory. Give an TodoItem and it will
 * malloc a new Line_t and add it to the linked list. Note that the function
 * does not render a new window for it. */
void line_list_add_new_item(TodoItem_t *item) {

  DEBUG("Appending new item %s", item->str);
  LineList_t *list = scrn->lines;
  Line_t *newLine = malloc(sizeof(Line_t));
  newLine->item = item;
  if (!newLine) {
    DEBUG("Malloc failed...");
    return;
  }
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

/* void ui_todo_toggle(Line_t *line) {} */

/* This will print the todo item str text stored in TodoItem->str on the
associated WINDOW of each Line_t.
* Also colours line based on completion status.
 * Used to render a line onto the screen when a new item is created.
*/
void line_render(Line_t *line, size_t row) {

  DEBUG("--> Rendering line in row %zu ", row);
  line->window = newwin(1, COLS - PADDING_X - 1, row, PADDING_X);

  DEBUG("Drawing '%s' onto the screen.", line->item->str);

  /* Check status value of each item->and apply attributes accordingly */
  if (line->item->status == e_status_complete) {
    DEBUG("Item is currently done, toggling into not done...");
    wattron(line->window, ATTR_DONE);
    line->item->status = e_status_complete;
    /* wprintw(line->window, "%s", line->item->str); */
    line_wprint(line);
    wrefresh(line->window);

  } else if (line->item->status == e_status_incomplete) {
    DEBUG("Item is currently done, toggling into not done...");
    /* NOTE: We can add highlighting here if we want to later on. */
    /* wattron(line->window, ATTR_TODO); */
    line->item->status = e_status_incomplete;
    /* wprintw(line->window, "%s", line->item->str); */
    line_wprint(line);
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
    DEBUG("Initialising item->'%zu', '%s', length: '%zu'", i, curr->item->str,
          curr->item->length);
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

bool scrn_lines_empty(void) {

  DEBUG("%s", "Testing if scrn->lines is empty.");

  if (scrn->lines->size == 0) {
    DEBUG("%s", "scrn->lines->size = 0.");
    return true;
  }
  if (scrn->lines->head == NULL) {
    DEBUG("%s", "scrn->lines->head is NULL.");
    return true;
  }
  return false;
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
  ui_hl_update(scrn->lines->current_line, NULL);

  return scrn;
}
