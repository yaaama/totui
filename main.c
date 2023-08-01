#include "item.h"
#include "ui.h"
#include <assert.h>
#include <ctype.h>
#include <curses.h>
#include <dialog.h>
#include <form.h>
#include <menu.h>
#include <ncurses.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define _GNU_SOURCE

char todo_file_name[64];

void setup_logging(char *file);
void append_to_todo_file(char *str);
void add_new_todo(Screen_t *scrn);
void init_todo_from_file(char *file);
void init_display_items_todo_window(void);
void handle_key_event(Screen_t *scrn, char key);

void nonkey_pressed(int keycode) {
  if ((keycode & 0x1f) ==
      keycode) { // Check if the keycode is a control character (0x1f = 31,
                 // ASCII control character range)
    DEBUG("'C-%c' pressed.",
          keycode + 'a' - 1); // Convert control keycode to corresponding letter
                              // (e.g., 3 => C-c)
  } else {
    DEBUG("'%c' pressed.",
          keycode); // Print the regular character if it's not a control key
  }
}

void todo_window_loop(Screen_t *scrn) {

  char key;

  while (true) {
    key = wgetch(scrn->currLine->window);

    switch (key) {
    case 'q' | 'Q': {
      /* Exit program here */
      DEBUG("---> User pressed '%c', quiting...", key);
      ui_destroy(scrn);
      exit(0);
      break;
    }
    case 'a': {
      DEBUG("User wants to add a new file...");
      add_new_todo(scrn);
      /* TODO Menu to ask them what to do */
      /* new_todo_handler(); */
      /* Refresh screen */
      break;
    }
    case 'd': {
      /* Delete todo */
    }

      /* Movement keys */
    case 'k': {
      ui_mv_cursor(scrn, e_mv_down);
      break;
    }
    case 'j': {
      ui_mv_cursor(scrn, e_move_down);
      break;
    }
    default:
      nonkey_pressed(key);
      break;
    }
  }
}

void add_new_todo(Screen_t *scrn) {

  Dimensions_t dim = {COLS / 2, LINES / 2};

  dialog_inputbox("New todo!", "Enter new item:", dim.y, dim.x, "", 0);
  dialog_vars.dlg_clear_screen = true;
  end_dialog();

  /* char *inp = dialog_vars.input_result; */
  char *inp = "Hey whaaaz up!";

  if (strlen(inp) == 0 || inp == NULL) {
    DEBUG("Empty input.");
    dlg_clr_result();
    /* Refresh screen */
    return;
  }

  /* Work out whether the string will actually be within the limits set out in
   * MAX_TODO_LEN */
  size_t inpLen = strlen(inp);
  DEBUG("Input length is: %zu", inpLen);
  size_t finalLen = inpLen + 6;
  DEBUG("Final formatted length is: %zu", finalLen);
  if (finalLen > MAX_TODO_LEN) {
    DEBUG("Input string is too long!");
  }

  char *fmt = calloc(MAX_TODO_LEN, sizeof(char)); /* New formatted string */
  const char *prepend = "[ ] ";
  strncat(fmt, prepend, 4); /* Inserting prepend first */

  strncat(fmt, inp, MAX_TODO_LEN);
  strncat(fmt, "\0", 2);

  DEBUG("Adding new todo item: '%s'", fmt);
  append_to_todo_file(fmt);

  TodoItem_t item;
  item.length = strlen(fmt);
  item.status = e_status_incomplete;
  strncpy(item.str, fmt, MAX_TODO_LEN);

  /* item.length = ; */
  /* Add the new item to the todo items list */

  /* init_display_items_todo_window(); */

  dlg_clr_result();
  /* Refresh screen */

  /* todoPane.totalCells ++; */
}

void append_to_todo_file(char *str) {

  FILE *fp = fopen(todo_file_name, "a");

  assert(fp != NULL);

  fprintf(fp, "%s\n", str);

  assert(str != NULL);
  fclose(fp);
}

/* Loads a file with a given name @fn
  MALLOC used */
Line_t **load_todo_file(char *fn) {

  DEBUG("Reading from file '%s'", fn);

  strcpy(todo_file_name, fn);

  assert(fn != NULL);
  /* TODO: init todo_file with name of file @fn */

  size_t currLen = 0;
  size_t index = 0;
  char currLine[128];
  FILE *fp = fopen(fn, "r");
  Line_t list[MAX_TODO_ITEMS];
  Line_t **retList;

  /* TODO Check if filename is real */

  assert(fp != NULL);

  while (fgets(currLine, MAX_TODO_LEN, fp) != NULL) {

    if (!(index < MAX_TODO_ITEMS)) {
      /* TODO Handle this error properly */
      /* Print error to log file */

      fclose(fp);
      return NULL;
    }

    currLen = strlen(currLine);

    Line_t nl = {0};

    /* Copying over string read from file */
    TodoItem_t tdItem = {0};
    strncpy(tdItem.str, currLine, 63);
    tdItem.str[strcspn(tdItem.str, "\n")] = 0;
    tdItem.str[currLen - 1] = '\0';

    tdItem.length = currLen;
    nl.item = tdItem;

    list[index] = nl;

    index++;
    DEBUG("Line '%s' loaded", nl.item.str);
  }

  char *filename = fn;
  /* Initialising global variables */
  initial_lines_c = index;
  char todo_file_name[64];
  strncpy(todo_file_name, filename, 64 - 1);
  todo_file_name[63] = '\0';

  /* Filling retList with the loaded data */
  retList = malloc(sizeof(Line_t *) * index);
  for (size_t i = 0; i < index; i++) {
    retList[i] = malloc(sizeof(Line_t));
    memcpy(retList[i], &list[i], sizeof(Line_t));
  }

  /* Closing the file */
  fclose(fp);

  DEBUG("---> %zu lines loaded from file '%s'", initial_lines_c,
        todo_file_name);
  return retList;
}

/*

 +-------------------------------+ <-- win_body
 |+-----------------------------+|
 ||                             ||
 ||                             ||
 ||          win_form           ||
 ||                             ||
 ||                             ||
 |+-----------------------------+|
 |+-----------------------------+|
 ||          win_menu           ||
 |+-----------------------------+|
 +-------------------------------+

 */

WINDOW *win_body, *win_form, *win_menu;
FORM *form;
FIELD **fields;
MENU *menu;
ITEM **items;
bool is_on_button; // Used to know the "case" we're in

void new_popup(int rows, int cols, int posy, int posx, char **buttons,
               int nb_buttons, char **requests, int nb_fields) {
  int i, cury = 0, curx = 1, tmp;
  WINDOW *inner;

  win_body = newwin(rows, cols, posy, posx);
  assert(win_body != NULL);
  box(win_body, 0, 0);

  items = malloc(sizeof(ITEM *) * (nb_buttons + 1));
  assert(items);

  for (i = 0; i < nb_buttons; i++) {
    items[i] = new_item(buttons[i], "");
    assert(items[i] != NULL);
  }

  items[i] = NULL;
  menu = new_menu(items);
  win_menu = derwin(win_body, 3, cols - 2, rows - 4, 1);
  assert(menu != NULL && win_menu != NULL);
  box(win_menu, 0, 0);
  set_menu_win(menu, win_menu);
  set_menu_format(menu, 1, nb_buttons);
  tmp = menu->fcols * (menu->namelen + menu->spc_rows);
  tmp--;
  inner = derwin(win_menu, 1, tmp, 1, (cols - 3 - tmp) / 2);
  assert(inner != NULL);
  set_menu_sub(menu, inner);
  set_menu_mark(menu, "");

  fields = malloc(sizeof(FIELD *) * (nb_fields + 1));
  assert(fields);

  for (i = 0; i < nb_fields; i++) {
    fields[i] = new_field(1, 10, cury, curx, 0, 0);
    assert(fields[i] != NULL);
    set_field_buffer(fields[i], 0, requests[i]);

    if (i % 2 == 1) {
      cury = cury + 1;
      curx = 1;
      field_opts_on(fields[i], O_ACTIVE);
      field_opts_on(fields[i], O_EDIT);
      set_field_back(fields[i], A_UNDERLINE);
    } else {
      curx = 20;
      field_opts_off(fields[i], O_ACTIVE);
      field_opts_off(fields[i], O_EDIT);
    }
  }

  fields[i] = NULL;
  form = new_form(fields);
  assert(form != NULL);
  win_form = derwin(win_body, rows - 5, cols - 2, 1, 1);
  box(win_form, 0, 0);
  assert(form != NULL && win_form != NULL);
  set_form_win(form, win_form);
  inner = derwin(win_form, form->rows + 1, form->cols + 1, 1, 1);
  assert(inner != NULL);
  set_form_sub(form, inner);

  assert(post_form(form) == E_OK);
  assert(post_menu(menu) == E_OK);
  is_on_button = true;
  pos_menu_cursor(menu);
}

void delete_popup(void) {
  int i;

  unpost_form(form);
  unpost_menu(menu);

  for (i = 0; fields[i] != NULL; i++) {
    free_field(fields[i]);
  }

  for (i = 0; items[i] != NULL; i++) {
    free_item(items[i]);
  }

  free_menu(menu);
  free_form(form);
  delwin(win_form);
  delwin(win_menu);
  delwin(win_body);
}

/*
 * Actions for 'return' on a button
 */
void driver_buttons(ITEM *item) {
  const char *name = item_name(item);
  int i;

  if (strcmp(name, "OK") == 0) {
    mvprintw(LINES - 2, 1, "[*] OK clicked:\t");

    for (i = 0; i < form->maxfield; i++) {
      printw("%s", field_buffer(fields[i], 0));

      if (field_opts(fields[i]) & O_ACTIVE)
        printw("\t");
    }

  } else if (strcmp(name, "QUIT") == 0)
    mvprintw(LINES - 2, 1, "[*] QUIT clicked, 'F1' to quit\n");

  refresh();
}

/*
 * When you want to change between the form and the buttons
 */
void switch_to_buttons(void) {
  // Those 2 lines allow the field buffer to be set
  form_driver(form, REQ_PREV_FIELD);
  form_driver(form, REQ_NEXT_FIELD);

  menu_driver(menu, REQ_FIRST_ITEM);
  is_on_button = true;
  set_menu_fore(menu, A_REVERSE); // "show" the button
}

void driver(int ch) {
  switch (ch) {
  case KEY_DOWN:
    if (is_on_button)
      break;

    if (form->current == fields[form->maxfield - 1])
      switch_to_buttons();
    else
      form_driver(form, REQ_NEXT_FIELD);
    break;

  case KEY_UP:
    if (is_on_button) {
      is_on_button = false;
      set_menu_fore(menu, A_NORMAL); // "hide" the button
    } else
      form_driver(form, REQ_PREV_FIELD);
    break;

  case KEY_LEFT:
    if (is_on_button)
      menu_driver(menu, REQ_LEFT_ITEM);
    else
      form_driver(form, REQ_LEFT_FIELD);
    break;

  case KEY_RIGHT:
    if (is_on_button)
      menu_driver(menu, REQ_RIGHT_ITEM);
    else
      form_driver(form, REQ_RIGHT_FIELD);
    break;

  case 10:
    if (!is_on_button)
      switch_to_buttons();
    else
      driver_buttons(current_item(menu));

    break;

  default:
    if (!is_on_button)
      form_driver(form, ch);

    break;
  }

  if (is_on_button)
    pos_menu_cursor(menu);
  else
    pos_form_cursor(form);

  wrefresh(win_body);
}

int createForm(void) {}

int main(void) {

  char *buttons[] = {"OK", "QUIT"};
  char *requests[] = {"Password:", "pass", "Id:", "id"};
  int ch;

  initscr();
  noecho();
  cbreak();
  keypad(stdscr, TRUE);

  new_popup(24, 80, (LINES - 25) / 2, (COLS - 81) / 2, buttons, 2, requests, 4);
  refresh();
  wrefresh(win_body);
  wrefresh(win_form);
  wrefresh(win_menu);

  while ((ch = getch()) != KEY_F(1))
    driver(ch);

  delete_popup();
  endwin();

  return 0;
  /* createForm(); */

  /* setup_logging(LOG_FILE); */

  /* Screen_t *scrn = ui_init(load_todo_file("example.txt")); */

  /* todo_window_loop(scrn); */

  /* system("sleep 1000"); */

  /* return 0; */
}

void setup_logging(char *file) {

  FILE *fptr = fopen(file, "w");

  if (fptr != NULL) {
    fclose(fptr);
  }
}
