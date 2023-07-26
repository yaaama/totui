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

/*******************/
/* /\* Typedef *\/ */
/*******************/
void append_to_file(char *file, char *str);
void new_todo_handler(void);
void init_todo_from_file(char *file);
void init_display_items_todo_window(void);

void setup_logging(char *file) {

  FILE *fptr = fopen(file, "w");

  if (fptr != NULL) {
    fclose(fptr);
  }
}

void print_msg(int code) {
  switch (code) {
  case 0:
    printf("Bye bye friend!\n");
    endwin();
    clear();
    exit(0);
    break;
  case 1:

    perror("Buffer overflow. Input is too big\n");
    endwin();
    exit(1);
    break;
  }
}

void todo_window_loop(Screen_t *scrn) {

  char key;

  while (true) {
    key = wgetch(scrn->currLine->ui_line);

    if (key == 'q' || key == 'Q') {
      /* Exit program here */
      exit(0);
    }

    if (key == 'a') {
      /* TODO Menu to ask them what to do */
      new_todo_handler();
                                /* Refresh screen */

    } else {
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
    append_to_file("example.txt", inp);

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
Line_t **load_file(char *fn) {

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
    DEBUG("String  \"%s\" loaded from file", nl.str);
  }

  char *filename = fn;
  /* Initialising global variables */
  initial_lines_c = index;
  char todo_file_name[64];
  strncpy(todo_file_name, filename, 64 - 1);
  todo_file_name[63] = '\0';

  DEBUG("initial_file_lines_count: %zu, todo_file: %s", initial_lines_c,
        todo_file_name);

  /* Filling retList with the loaded data */
  retList = malloc(sizeof(Line_t *) * index);
  for (size_t i = 0; i < index; i++) {
    retList[i] = malloc(sizeof(Line_t));
    memcpy(retList[i], &list[i], sizeof(Line_t));
  }

  /* Closing the file */
  fclose(fp);
  return retList;
}

int main(void) {

  setup_logging(LOG_FILE);

  Screen_t *scrn = ui_init(load_file("example.txt"));

  getch();


  todo_window_loop(scrn);

  /* system("sleep 1000"); */

  return 0;
}
