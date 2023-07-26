#include "item.h"
#include "ui.h"
#include <assert.h>
#include <ctype.h>
#include <curses.h>
#include <dialog.h>
#include <ncurses.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define _GNU_SOURCE

char todo_file_name[64];

enum KEY_EVENT { UP, DOWN, CREATE, DELETE };

void setup_logging(char *file) {

  FILE *fptr = fopen(file, "w");

  if (fptr != NULL) {
    fclose(fptr);
  }
}

void append_to_todo_file(char *str);
void new_todo_handler(void);
void init_todo_from_file(char *file);
void init_display_items_todo_window(void);
void handle_key_event(Screen_t *scrn, enum KEY_EVENT key);

void handle_key_event(Screen_t *scrn, enum KEY_EVENT key) {

  switch (key) {

  case UP:
  case DOWN:
  case CREATE:
  case DELETE:
    break;
  }
}

void todo_window_loop(Screen_t *scrn) {

  char key;

  while (true) {
    key = wgetch(scrn->currLine->ui_line);

    switch (key) {
    case 'q' | 'Q': {
      /* Exit program here */
      exit(0);
      break;
    }
    case 'a': {
      DEBUG("User wants to add a new file...");
      /* TODO Menu to ask them what to do */
      /* new_todo_handler(); */
      /* Refresh screen */
      break;
    }
    case 'k': {
      handle_key_event(scrn, DOWN);
    }
    }
  }
}

void new_todo_handler(void) {

  dialog_inputbox("New todo!", "Enter new item:", 20, 50, "", 0);
  dialog_vars.dlg_clear_screen = true;
  end_dialog();
  char *inp = dialog_vars.input_result;

  if (strlen(inp) == 0 || inp == NULL) {
    perror("Empty input.");
    dlg_clr_result();
    /* Refresh screen */

    return;
  } else {
    append_to_todo_file(inp);

    /* Add the new item to the todo items list */

    /* init_display_items_todo_window(); */
    dlg_clr_result();
    /* Refresh screen */

    /* todoPane.totalCells ++; */
  }
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

  DEBUG("Reading from file '%s'" , fn);

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
    strncpy(nl.str, currLine, 63);
    nl.str[strcspn(nl.str, "\n")] = 0;
    nl.str[currLen - 1] = '\0';

    nl.length = currLen;

    list[index] = nl;

    index++;
    DEBUG("Line '%s' loaded", nl.str);
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

  Screen_t *scrn = ui_init(load_todo_file("example.txt"));

  todo_window_loop(scrn);

  /* system("sleep 1000"); */

  return 0;
}
