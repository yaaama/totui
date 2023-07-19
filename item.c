#include "item.h"
#include <assert.h>

/* Changes the status of an item */
void change_item_status(todo_item *item, ITEM_STATUS s) {

  assert(item != NULL);

  item->status = s;
}

/* Prints the value of an item */
void print_item(todo_item *item) {

  assert(item != NULL);

  printf("Item head: %s , item status: %s\n", item->heading,
         status_string(item->status));
}

/* Converts enum value into a string */
const char *status_string(ITEM_STATUS s) {
  switch (s) {
  case COMPLETE:
    return "COMPLETE";
    break;
  case PROGRESSING:
    return "PROGRESSING";
    break;
  case NOTSTARTED:
    return "NOTSTARTED";
    break;
  default:
    return NULL;
  }
}
