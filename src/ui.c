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
void line_refresh(Line_t *line);
void draw_mainscrn_box(WINDOW *window);
void ui_terminal_resized(void);
int render_all_lines(LineList_t *lines);
void refresh_all_lines(LineList_t *lines);
void init_echo_bar(void);
void echo_to_user(char *msg);
void clear_echo_bar(void);
void draw_echo_box(WINDOW *win);

bool window_too_small(void) {

  getmaxyx(scrn->main, scrn->dimen.y, scrn->dimen.x);
  DEBUG("Window size is currently %zu x %zu", scrn->dimen.x, scrn->dimen.y);

  if (scrn->dimen.x < MAX_TODO_LEN) {
    DEBUG("This window is too small!");
    return true;
  }

  return false;
}

void display_window_too_small_err(void) {

  bool small = window_too_small();

  char *errmsg = "Window is too small!";

  size_t xval = getmaxx(scrn->main);
  size_t centerX = (xval / 2) - (strlen(errmsg) / 2);

  erase();
  mvwprintw(scrn->main, 1, centerX, "%s", errmsg);

  do {

    int chr = wgetch(stdscr);
    DEBUG("Window is currently: %zu cols wide", scrn->dimen.x);

    if (chr == KEY_RESIZE) {
      small = window_too_small();
    } else if (chr == 'q') {
      ui_destroy();
      exit(0);
    }

  } while (small == true);
  erase();

  refresh();
  draw_mainscrn_box(scrn->main);
  refresh_all_lines(scrn->lines);
}

void display_warning(ERROR_T error) {

  switch (error) {

  case err_term_small: {
    display_window_too_small_err();
    break;
  }
  case err_no_items:
    break;
  }
}

void ui_terminal_resized(void) {

  getmaxyx(scrn->main, scrn->dimen.y, scrn->dimen.x);
  DEBUG("Terminal has been resized.");

  if (scrn->dimen.x < MAX_TODO_LEN) {
    DEBUG("Terminal is too small...");
    display_warning(err_term_small);
  }

  /* endwin(); */
  werase(scrn->main);

  wresize(scrn->main, scrn->dimen.y, scrn->dimen.x);

  draw_mainscrn_box(scrn->main);

  refresh();
  /* render_all_lines(scrn->lines); */
  refresh_all_lines(scrn->lines);
  echo_to_user("Terminal resized!");
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

  if (scrn->main) {
    delwin(scrn->main);
  }

  if (scrn->echo_bar) {
    delwin(scrn->echo_bar);
  }

  if (scrn->help_bar) {
    delwin(scrn->help_bar);
  }

  free(scrn->lines);
  free(scrn);

  endwin();
  refresh();
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
  werase(line->window);
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
}

/* This function moves the cursor either up or down. */
void ui_mv_cursor(MOVEMENT_TYPE_e go) {

  /* Cant move a cursor if there are no lines. */
  if (scrn->lines->size == 0) {
    DEBUG("--> Screen empty, movement is impossible.");
    return;
  }

  Line_t *mvHere = NULL;
  Line_t *currLine = scrn->lines->current_line;

  /* If the current line is null for whatever reason, then scrn->current_line
   * should be replaced with the first line. */
  if (currLine == NULL) {
    scrn->lines->current_line = scrn->lines->head;
  }

  clear_echo_bar();

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

  size_t maxY, maxX;
  getmaxyx(scrn->main, maxY, maxX);
  DEBUG("Window has been resized! New dimensions: (%zu, %zu)", maxX, maxY);
  /* resizeterm(maxY, maxX); */
  /* wresize(scrn->main, LINES, COLS); */
  box(scrn->main, maxY, maxX);
  wclear(scrn->main);
  refresh();
  wrefresh(scrn->main);
}

/* ui_refresh refreshes the screen. */
void ui_refresh(void) {

  // TODO Add resize handling here.
  ui_resized();
  /* if (is_term_resized(int, int)) { */
  /*   ui_resized(); */
  /* } */

  // Why bother refreshing an empty screen?
  if (scrn->lines->size == 0) {
    DEBUG("No lines to refresh, returning...");
    return;
  }

  /* If todo list is just 1 item then hl that 1 item. */
  if (scrn->lines->size == 1) {
    ui_hl_update(scrn->lines->current_line, NULL);
  }

  DEBUG("Refreshing all %zu lines!", scrn->lines->size);

  werase(scrn->main);
  draw_mainscrn_box(scrn->main);
  wrefresh(scrn->main);

  // Start refreshing from the start (the head) of the list
  Line_t *curr = scrn->lines->head;

  while (curr) {
    line_refresh(curr);
    curr = curr->next;
  }

  ui_hl_update(scrn->lines->current_line, NULL);

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
  size_t currY;

  while (curr) {
    DEBUG("Attempting to redraw item-> %s", curr->item->str);
    currY = curr->window->_begy;

    // All lines below the deleted line will be shifted up
    if (currY >= delWinY) {
      line_render(curr, newY);
      curr = curr->next;
      newY++;
      continue;
    }

    // No need to rerender the line if it is not being shifted
    line_refresh(curr);
    curr = curr->next;
  }
}

/* Append a new Line_t to the scrn->lines linked list.
 * This function should be self explanatory. Give an TodoItem and it will
 * malloc a new Line_t and add it to the linked list. Note that the function
 * does not render a new window for it. */
void linelist_add_item(TodoItem_t *item) {

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

void line_refresh(Line_t *line) {
  werase(line->window);
  wmove(line->window, 0, PADDING_X);
  line_wprint(line);
  wrefresh(line->window);
}

void refresh_all_lines(LineList_t *lines) {

  Line_t *curr = lines->head;

  while (curr) {
    /* werase(curr->window); */
    wresize(curr->window, 1, scrn->dimen.x - PADDING_X - 1);
    wmove(curr->window, 0, PADDING_X);
    line_wprint(curr);
    wnoutrefresh(curr->window);
    curr = curr->next;
  }
  doupdate();
  ui_hl_update(lines->current_line, NULL);
}

/* Returns current dimensions of the main screen */
Dim_t curr_dim(void) {
  Dim_t dim;
  getmaxyx(scrn->main, dim.y, dim.x);
  return dim;
}

/* This will print the todo item str text stored in TodoItem->str on the
associated WINDOW of each Line_t.
* Also colours line based on completion status.
 * Used to render a line onto the screen when a new item is created.
*/
void line_render(Line_t *line, size_t row) {

  size_t startX = scrn->dimen.x - PADDING_X - 1;
  line->window = newwin(1, startX, row, PADDING_X);

  DEBUG("Rendering row %zu with '%s' onto the screen.", row, line->item->str);

  /* Check status value of each item->and apply attributes accordingly */
  if (line->item->status == e_status_complete) {
    wattron(line->window, ATTR_DONE);

  } else if (line->item->status == e_status_incomplete) {
    /* NOTE: We can add highlighting here if we want to later on. */
    /* wattron(line->window, ATTR_TODO); */
  }

  line_refresh(line);
}

/* Called by ui_init_screen
 * Takes in a LineList_t and renders all of the Lines. */
int render_all_lines(LineList_t *list) {

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

  return i;
}

void clear_up_terminal_window(void) {}

void draw_mainscrn_box(WINDOW *window) { box(window, ACS_VLINE, ACS_HLINE); }

/* Draws up the main window stored in scrn->main */
void init_main_screen(void) {

  ui_init_colours();
  DEBUG("Initialising scrn->main with dimensions: [%zu x %zu]", scrn->dimen.x,
        scrn->dimen.y);
  /* TODO Initialise echo bar and help bar */

  // Draws a box
  draw_mainscrn_box(scrn->main);

  if (window_too_small()) {
    char *errmsg = "Please restart Totui using a larger terminal window!";
    delwin(scrn->main);
    endwin();
    refresh();

    printf("\n------");
    printf("\n\nERROR!!\n%s\n", errmsg);
    printf("\n------\n");

    exit(1);
  }

  wrefresh(scrn->main);
  scrn->current_line_index = 0;
}

void draw_echo_box(WINDOW *win) { box(win, 0, 0); }

void clear_echo_bar(void) {

  werase(scrn->echo_bar);
  draw_echo_box(scrn->echo_bar);

  wrefresh(scrn->echo_bar);
}

/* Initialises the echo bar */
void init_echo_bar(void) {

  size_t startX = scrn->dimen.x;
  size_t yVal = scrn->dimen.y - 3;
  char *loadstatus = "Totui has succesfully loaded!";
  size_t lsSize = strlen(loadstatus);
  // To center the initial load up message
  size_t centerX = (scrn->dimen.x / 2) - (lsSize / 2);

  scrn->echo_bar = newwin(3, startX, yVal, 0);
  draw_echo_box(scrn->echo_bar);
  wattron(scrn->echo_bar, A_DIM | A_BOLD | COLOR_PAIR(1));
  mvwprintw(scrn->echo_bar, 1, centerX, "%s", loadstatus);
  wattroff(scrn->echo_bar, A_DIM | A_BOLD | COLOR_PAIR(1));

  wrefresh(scrn->echo_bar);
}

void echo_to_user(char *msg) {

  size_t msgLen = strlen(msg);
  size_t centerX = (scrn->dimen.x / 2) - (msgLen / 2);

  werase(scrn->echo_bar);
  wattron(scrn->echo_bar, A_DIM | A_BOLD | COLOR_PAIR(1));
  mvwprintw(scrn->echo_bar, 1, centerX, "%s", msg);
  wattroff(scrn->echo_bar, A_DIM | A_BOLD | COLOR_PAIR(1));
  draw_echo_box(scrn->echo_bar);
  wrefresh(scrn->echo_bar);
}

/* Called when the program starts
 * Initialises the screen and sets some ncurses options whilst also passing the
 * LineList to get drawn  */
Screen_t *ui_init(LineList_t *lines) {

  // Scrn is where we will store the system state
  scrn = malloc(sizeof(Screen_t));

  // Initialising ncurses
  scrn->main = initscr();
  cbreak();
  noecho();
  raw();
  curs_set(0);
  nonl(); /* Controls where ENTER key is drawn onto page */
  keypad(scrn->main, TRUE);

  // Dimensions of screen initially
  getmaxyx(stdscr, scrn->dimen.y, scrn->dimen.x);
  init_main_screen();

  // Set up lines linked list
  int loaded = render_all_lines(lines);

  // If file is not empty then we should highlight the first line.
  if (loaded > 0) {
    ui_hl_update(scrn->lines->current_line, NULL);
  }

  init_echo_bar();

  return scrn;
}
