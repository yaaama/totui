#ifndef ITEM_H_
#define ITEM_H_

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/* Enum that stores the status of the todo item */
typedef enum { COMPLETE, PROGRESSING, NOTSTARTED } ITEM_STATUS;

/* TODO */
typedef struct {
  char *text;
} item_text;

/* Todo item */
typedef struct {
  char *heading;
  ITEM_STATUS status;
  /* void *text; */
  /* bool completed; */
} todo_item;

void change_item_status(todo_item *item, ITEM_STATUS s);

void print_item(todo_item *item);

const char *status_string(ITEM_STATUS s);
#endif // ITEM_H_
