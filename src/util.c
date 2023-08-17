#include "util.h"

/**********************************/
/* /\* Functions declarations *\/ */
/**********************************/
const char *get_local_time(void);
void print_all_todo_items(LineList_t *line_list);
void append_to_file(char *filename, char *str);
char *strip_string(char *str);

/************************************/
/* /\* Function implementations *\/ */
/************************************/

/*
 * 1: General utility functions.
 * 2: General file handling functions.
 * 3: Specialised functions dealing with
 special types, etc.
 */

/*****/
/* 1 */
/*****/

/* Returns local time. */
const char *get_local_time(void) {
  time_t rawtime;
  struct tm *timeinfo;
  static char buffer[80];
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  strftime(buffer, 80, "%H:%M:%S", timeinfo);
  return buffer;
}

/* Opens/creates the file we will be outputting log messages to */
void setup_log_file(char *file) {

  FILE *fptr = fopen(file, "w");

  if (fptr != NULL) {
    fclose(fptr);
  }
}

/*****/
/* 2 */
/*****/

/* Opens the todo file on system and appends a new item to it */
void append_to_file(char *filename, char *str) {

  FILE *fp = fopen(filename, "a");

  assert(fp != NULL);

  fprintf(fp, "%s\n", str);

  assert(str != NULL);
  fclose(fp);
}

/*****/
/* 3 */
/*****/

/* Loads a file with a given name filename (fn)
  Returns @LineList_t containing @Line_t items
  @LineList_t initialises `size`, `head`, `tail`, `current_line`
  Each @Line_t has its `item.str`, `item.length` initialised
  MALLOC used */
LineList_t *load_todo_file(char *fn) {

  DEBUG("Reading from file '%s'", fn);

  assert(fn != NULL && "File name was NULL!");

  FILE *fp = fopen(fn, "r");
  assert(fp != NULL && "File could not be opened for whatever reason.");

  todo_file_name = fn; /* Initialising the global filename var */

  char currLine[MAX_TODO_LEN];
  Line_t *prev = NULL;
  LineList_t *retList = malloc(sizeof(LineList_t));

  if (!retList) {
    fclose(fp);
    return NULL;
  }

  retList->head = NULL;
  retList->tail = NULL;
  retList->current_line = NULL;
  retList->size = 0;

  size_t index = 0;

  /* Reading the file now... */
  while (fgets(currLine, MAX_TODO_LEN, fp) != NULL) {

    if (!(index < MAX_TODO_ITEMS)) {
      fclose(fp);
      linelist_destroy(retList); /* Destroys the linked list structure */
      return NULL;
    }

    Line_t *nl = malloc(sizeof(Line_t));
    if (!nl) {
      fclose(fp);
      DEBUG("Error allocating a line. %s", "Returning NULL.");
      linelist_destroy(retList);
      return NULL;
    }

    // Initialize the Line node
    nl->next = NULL;
    nl->previous = prev;

    /* REVIEW This may cause some problems later on... */
    char *fmtCurrLine = strip_string(currLine);

    TODO_STATUS_t status = parse_todo_status(fmtCurrLine);
    nl->item.status = status; /* Setting the status */

    strncpy(nl->item.str, fmtCurrLine, MAX_TODO_LEN - 1); /* Sets the raw str */

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

/* Taken from stack overflow:
 * https://stackoverflow.com/questions/352055/best-algorithm-to-strip-leading-and-trailing-spaces-in-c?noredirect=1&lq=1
 */
char *strip_string(char *s) {
  size_t size;
  char *end;

  size = strlen(s);

  if (!size)
    return s;

  end = s + size - 1;
  while (end >= s && isspace(*end))
    end--;
  *(end + 1) = '\0';

  while (*s && isspace(*s))
    s++;

  return s;
}

void format_current_line(char *str) {

  while (isspace(*str)) {
  }
}

/* Takes a string and determines if the line is done or not */
TODO_STATUS_t parse_todo_status(char *str) {

  DEBUG("%s %s", "---> Parsing the todo status of line:", str);
  /* List of things to look for */
  const char *done = ":DONE:";
  const char *todo = ":TODO:";
  /* char *done = ":DONE:"; */

  if (strstr(str, done)) {
    return e_status_complete;
  } else if (strstr(str, todo)) {
    return e_status_incomplete;
  }

  assert(false && "TODO Checkbox not found. Handle this.");

  return e_status_incomplete;
}

/* Prints all the strings for each todo item stored in the linked list  */
void print_all_todo_items(LineList_t *line_list) {

  DEBUG("--> Printing all stored todo items in LineList");

  if (!line_list->head) {
    DEBUG("%s", "List empty!");
    return;
  }

  Line_t *curr = line_list->head;
  size_t count = 0;
  while (curr) {
    DEBUG("%zu : %s", count, curr->item.str);
    ++count;
    curr = curr->next;
  }
}
