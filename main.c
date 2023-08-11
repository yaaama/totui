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
    key = wgetch(scrn->lines->current_line->window);

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

    DEBUG("%s", "Empty input.");
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
    DEBUG("%s", "Input string is too long!");
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
  line_append(item);
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

void linelist_init(LineList_t *list) {
  list->head = NULL;
  list->tail = NULL;
  list->current_line = NULL;
  list->size = 0;
}

/* Add a todo item to the end of the list */
void linelist_add_item(LineList_t *list, char *str, TODO_STATUS_t status) {
  Line_t *newNode = malloc(sizeof(Line_t));
  if (!newNode)
    return;

  newNode->item.length = strlen(str);
  strncpy(newNode->item.str, str, MAX_TODO_LEN);
  newNode->item.status = status;
  newNode->next = NULL;
  newNode->previous = list->tail;

  if (list->tail) {
    list->tail->next = newNode;
  } else {
    list->head = newNode;
  }

  list->tail = newNode;
  list->size++;
}

/* Delete a node from the list */
void linelist_remove_item(LineList_t *list, Line_t *node) {
  if (node->previous) {
    node->previous->next = node->next;
  } else {
    list->head = node->next;
  }

  if (node->next) {
    node->next->previous = node->previous;
  } else {
    list->tail = node->previous;
  }

  free(node);
  list->size--;
}

/* Find a node by todo string */
Line_t *linelist_find_item(LineList_t *list, char *str) {
  Line_t *curr = list->head;
  while (curr) {
    if (strncmp(curr->item.str, str, MAX_TODO_LEN) == 0) {
      return curr;
    }
    curr = curr->next;
  }
  return NULL;
}

/* Traverse and print the list for demonstration */
void linelist_print(LineList_t *list) {
  Line_t *curr = list->head;
  while (curr) {
    printf("Todo: %s\n", curr->item.str);
    curr = curr->next;
  }
}

void linelist_destroy(LineList_t *list) {
  Line_t *curr = list->head;
  while (curr) {
    Line_t *next = curr->next;
    free(curr);
    curr = next;
  }
}

/* Loads a file with a given name filename (fn)
  Returns @LineList_t containing @Line_t items
  @LineList_t initialises `size`, `head`, `tail`, `current_line`
  Each @Line_t has its `item.str`, `item.length` initialised
  MALLOC used */
LineList_t *load_todo_file(char *fn) {

  DEBUG("Reading from file '%s'", fn);

  strcpy(todo_file_name, fn);
  assert(fn != NULL && "File does not exist.");

  FILE *fp = fopen(fn, "r");
  assert(fp != NULL);

  char currLine[MAX_TODO_LEN];
  Line_t *prev = NULL;
  LineList_t *retList = malloc(sizeof(LineList_t));

  if (!retList) {
    fclose(fp);
    return NULL;
  }

  linelist_init(retList); // Use the function from the previous answer.

  size_t index = 0;
  while (fgets(currLine, MAX_TODO_LEN, fp) != NULL) {
    if (!(index < MAX_TODO_ITEMS)) {
      fclose(fp);
      linelist_destroy(retList); // Use the function from the previous answer.
      return NULL;
    }

    Line_t *nl = malloc(sizeof(Line_t));
    if (!nl) {
      fclose(fp);
      linelist_destroy(retList); // Use the function from the previous answer.
      return NULL;
    }

    // Initialize the node
    nl->next = NULL;
    nl->previous = prev;
    strncpy(nl->item.str, currLine, MAX_TODO_LEN - 1);
    nl->item.str[strcspn(nl->item.str, "\n")] = 0; // Remove newline
    nl->item.length = strlen(nl->item.str);

    // Add the node to the linked list
    if (prev) {
      prev->next = nl;
    } else {
      retList->head = nl;
    }
    prev = nl;
    index++;

    DEBUG("Line '%s' loaded", nl->item.str);
  }

  retList->tail = prev;
  retList->size = index;

  fclose(fp);

  DEBUG("---> %zu lines loaded from file '%s'", index, todo_file_name);
  retList->current_line = retList->head;
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
