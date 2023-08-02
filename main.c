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
Screen_t *scrn;

void setup_logging(char *file);
void append_to_todo_file(char *str);
void add_new_todo(void);
void init_todo_from_file(char *file);
void init_display_items_todo_window(void);
void handle_key_event(char key);
void todo_window_loop(void);

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

void todo_window_loop(void) {

  char key;

  while (true) {
    key = wgetch(scrn->currLine->window);

    switch (key) {
    case 'q' | 'Q': {
      /* Exit program here */
      DEBUG("---> User pressed '%c', quiting...", key);
      ui_destroy();
      exit(0);
      break;
    }
    case 'a': {
      DEBUG("%s", "User wants to add a new todo item...");
      /* add_new_todo(scrn); */

      add_new_todo();
      ui_refresh();

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
      ui_mv_cursor(e_mv_up);
      break;
    }
    case 'j': {
      ui_mv_cursor(e_mv_down);
      break;
    }
    default:
      nonkey_pressed(key);
      break;
    }
  }
}

void add_new_todo(void) {

  Dimensions_t dim = {COLS / 2, LINES / 2};

  dialog_inputbox("New todo!", "Enter new item:", dim.y, dim.x, "", 0);
  dialog_vars.dlg_clear_screen = true;

  char *inp = dialog_vars.input_result;
  end_dialog();

  DEBUG("Input received: %s", inp);
  if ((strlen(inp) == 0 || inp == NULL)) {

    DEBUG("Empty input.");
    endwin();
    clear();
    dlg_clr_result();
    dlg_clear();
    refresh();
    ui_refresh();
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

  /* Formatting the input received */
  char *fmt = calloc(MAX_TODO_LEN, sizeof(char));
  const char *prepend = "[ ] ";
  strncat(fmt, prepend, 4); /* Inserting prepend first */
  strncat(fmt, inp, MAX_TODO_LEN - 1);
  strncat(fmt, "\0", 2);

  DEBUG("Adding new todo item: '%s'", fmt);
  append_to_todo_file(fmt);

  TodoItem_t item;
  item.length = strlen(fmt);
  item.status = e_status_incomplete;
  strncpy(item.str, fmt, MAX_TODO_LEN);

  /* Add the new item to the todo items list */
  line_append(item, scrn->lines_total + 1);
  ui_refresh();

  dlg_clr_result();

  free(dialog_vars.input_result);

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

  assert(fn != NULL && "File does not exist.");
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

int main(void) {

  setup_logging(LOG_FILE);

  ui_init(load_todo_file("example.txt"));

  todo_window_loop();

  return 0;
}

void setup_logging(char *file) {

  FILE *fptr = fopen(file, "w");

  if (fptr != NULL) {
    fclose(fptr);
  }
}
