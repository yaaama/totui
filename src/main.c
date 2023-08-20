#include "ui.h"
#include "util.h"

/*********************************/
/* /\* Function declarations *\/ */
/*********************************/
void add_new_todo(void);
void init_todo_from_file(char *file);
void init_display_items_todo_window(void);
void handle_key_event(char key);
void todo_window_loop(void);
void delete_todo_item(void);
void linelist_remove_item(LineList_t *list, Line_t *node);
void toggle_todo_curr_item(void);

Screen_t *scrn; /* Global scrn var initialised in ui.c */
char *todo_file_name;

/* Main loop of the program.
 * Listens for a key pressed and then uses a switch to match it and act
 * appropiately. */
void todo_window_loop(void) {

  while (true) {
    fflush(stdin);

    char key = wgetch(scrn->main);

    switch (key) {
      /* Quit program */
    case 'q' | 'Q': {
      /* Exit program here */
      DEBUG("---> User pressed '%c', quiting...", key);
      dump_state(scrn->lines);
      ui_destroy();

      exit(0);
      break;
    }
      /* Toggle todo status */
    case ' ': {
      toggle_todo_curr_item();
      break;
    }
      /* Add new item */
    case 'a': {
      DEBUG("%s", "User wants to add a new todo item...");
      add_new_todo();

      break;
    }
      /* Remove item */
    case 'd': {
      /* Delete todo */
      delete_todo_item();
      break;
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
      /* TODO Handle when a non recognised key is pressed
       * nonkey_pressed(key); */
      break;
    }
  }
}

void toggle_todo_curr_item(void) {

  DEBUG("--> Toggling todo...");
  TODO_STATUS_e currStatus = scrn->lines->current_line->item->status;
  /* TODO Change the str value of the todo item to reflect the change in status
   */

  if (currStatus == e_status_complete) {
    /* currStatus = e_status_incomplete; */
    scrn->lines->current_line->item->status = e_status_incomplete;

  } else {
    /* *currStatus = e_status_complete; */
    scrn->lines->current_line->item->status = e_status_complete;
  }

  DEBUG("Row val: %zu", scrn->current_line_index + 1);
  line_render(scrn->lines->current_line, scrn->current_line_index + 1);
  ui_hl_update(scrn->lines->current_line, NULL);
}

/* Entry point for deletion
 * Displays a dialog window and asks if user really wants to delete item */
void delete_todo_item(void) {

  if (scrn_lines_empty()) {
    return;
  }

  int yn =
      dialog_yesno("Deleting todo item!", "Do you really want to delete this?",
                   LINES / 2, COLS / 2);

  if (yn) { /* User no longer wants to delete the item */
    /* Clear up the screen and refresh it */
    end_dialog();
    refresh();
    ui_refresh();
    return;
  }

  DEBUG("User wants to delete item '%s'", scrn->lines->current_line->item->str);

  /* This is the Y coord of the deleted window */
  size_t delY = scrn->lines->current_line->window->_begy;
  linelist_remove_item(scrn->lines, scrn->lines->current_line);
  scrn->current_line_index--;

  print_all_todo_items(scrn->lines);

  if (scrn_lines_empty()) {
    /* TODO Handle what happens when the screen is empty */
    /* ui_empty_todolist(); */
    end_dialog();
    refresh();
    ui_refresh();
    return;
  }
  end_dialog();
  refresh();
  ui_refresh_delete(delY);
  ui_hl_update(scrn->lines->current_line, NULL);
}

/* Entry point for adding a new todo item
 * Displays a dialog window and asks what the todo item should be called */
void add_new_todo(void) {

  Dimensions_t dim = {COLS / 2, LINES / 2};

  dialog_inputbox("New todo!", "Enter new item:", dim.y, dim.x, "", 0);
  dialog_vars.dlg_clear_screen = true;

  char *inp = dialog_vars.input_result;
  end_dialog();

  DEBUG("Input received: %s", inp);
  if ((strlen(inp) == 0)) {

    DEBUG("%s", "Empty input received.");
    /* Clear up the screen and refresh it */
    /* dlg_clr_result(); */
    /* dlg_clear(); */
    refresh();
    ui_refresh();
    return;
  }

  size_t inpLen = strlen(inp);
  DEBUG("Input length is: %zu", inpLen);

  /* Formatting the input received */
  char fmtedStr[MAX_TODO_LEN + 1] = {""};
  char *prepend = ":TODO: "; /* TODO: Change this to a macro or something. */
  size_t tagLen = strlen(prepend);
  strncat(fmtedStr, prepend, tagLen + 1);
  strncat(fmtedStr, inp, MAX_TODO_LEN - tagLen);
  TodoItem_t *item = malloc(sizeof(TodoItem_t));

  DEBUG("Adding new todo item: '%s'", fmtedStr);
  append_to_file(todo_file_name, fmtedStr);
  cut_tag_from_line_string(fmtedStr, e_status_incomplete);

  item->length = strlen(fmtedStr);
  item->status = e_status_incomplete;
  strncpy(item->str, fmtedStr, MAX_TODO_LEN);

  /* Add the new item to the todo items list */
  line_list_add_new_item(item);
  /* Render this new line */
  line_render(scrn->lines->tail, scrn->lines->size);

  refresh();
  ui_refresh();

  dlg_clr_result();

  free(dialog_vars.input_result);
  /* dlg_clear(); */
}

/* Add a todo item to the end of the linked list */
void linelist_add_item(LineList_t *list, char *str, TODO_STATUS_e status) {
  Line_t *newNode = malloc(sizeof(Line_t));

  if (!newNode)
    return;
  newNode->item = malloc(sizeof(TodoItem_t));

  newNode->item->length = strlen(str);
  strncpy(newNode->item->str, str, MAX_TODO_LEN);
  newNode->item->status = status;
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

  Line_t *newCurr;

  if (node->previous) {
    node->previous->next = node->next;
    newCurr = node->previous;
  } else {
    list->head = node->next;
    newCurr = node->next;
  }

  if (node->next) {
    node->next->previous = node->previous;

  } else {
    list->tail = node->previous;
  }

  delwin(node->window);
  free(node);
  list->size--;
  list->current_line = newCurr;
}

/* Find a node by todo string */
Line_t *linelist_find_item(LineList_t *list, char *str) {
  Line_t *curr = list->head;
  while (curr) {
    if (strncmp(curr->item->str, str, MAX_TODO_LEN) == 0) {
      return curr;
    }
    curr = curr->next;
  }
  return NULL;
}

/* Destroys the linked list.
 * This is only called BEFORE any of the list items have a window attached.
 * If there is a window in the list item, then you will have to call a destroy
 * function for the entirety of the ui (func located in the ui file somewhere,
 * it is easy to find) */
void linelist_destroy(LineList_t *list) {
  Line_t *curr = list->head;
  while (curr) {
    Line_t *next = curr->next;
    free(curr);
    curr = next;
  }
}

/* Main method that drives the entire program
  TODO Make this parse arguments at a later date */
int main(void) {

  setup_log_file(LOG_FILE);

  ui_init(load_todo_file("example.txt"));

  todo_window_loop();

  return 0;
}
