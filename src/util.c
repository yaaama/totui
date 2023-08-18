#include "util.h"

/**********************************/
/* /\* Functions declarations *\/ */
/**********************************/
const char *get_local_time(void);
void print_all_todo_items(LineList_t *line_list);
void append_to_file(char *filename, char *str);
char *strip_string(char *str);
char *status_enum_to_string(TODO_STATUS_e status);
char *handle_no_status(char *str);
void replace_tag_with_checkbox(char *str, TODO_STATUS_e status);
void cut_tag_from_line_string(char *str, TODO_STATUS_e status);

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

/* Strips a string of its trailing white space and its leading whitespace.
 * Taken from stack overflow:
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

    char fmtCurrLine[MAX_TODO_LEN + 1] = {""};
    DEBUG("-> 1 Current line: %s", currLine);
    strcpy(fmtCurrLine, currLine);
    strip_string(fmtCurrLine);
    DEBUG("-> 2 Formatted line: %s", fmtCurrLine);

    /* Figuring out what the status for the current line is */
    TODO_STATUS_e status = parse_todo_status(fmtCurrLine);

    if (status == e_status_none) {
      handle_no_status(fmtCurrLine);
      status = e_status_incomplete;
    }
    nl->item.status = status; /* Setting the status */

    cut_tag_from_line_string(fmtCurrLine, nl->item.status);
    DEBUG("-> 3 Removed tag: %s", fmtCurrLine);
    strncpy(nl->item.str, fmtCurrLine, MAX_TODO_LEN - 1); /* Sets the raw str */
    DEBUG("-> 4 Copied str: %s", nl->item.str);
    // REVIEW I decided to make not include this and actually have the checkbox
    // processed during UI rendering.
    /* // Replaces the status tag with a checkbox */
    /* append_box_to_file_str(nl->item.str, status); */

    nl->item.length = strlen(nl->item.str);

    // Add the node to the linked list
    if (prev) {
      prev->next = nl;
    } else {
      retList->head = nl;
    }
    prev = nl;
    index++;

    DEBUG("->3 Line '%s' loaded", nl->item.str);
  }

  retList->tail = prev;
  retList->size = index;

  fclose(fp);

  DEBUG("---> %zu lines loaded from file '%s'", index, todo_file_name);
  retList->current_line = retList->head;
  return retList;
}

void dump_todo_to_file(LineList_t *lines) {

  Line_t *curr = lines->head;

  while (curr) {
  }
}

/* This function will cut tag such as `T/ODO` or `DONE` from a string */
void cut_tag_from_line_string(char *str, TODO_STATUS_e status) {

  char retStr[MAX_TODO_LEN] = {""};

  char *statusStr = status_enum_to_string(status);
  DEBUG("--> Status str: %s", statusStr);
  size_t statusLen = strlen(statusStr);
  DEBUG("statuslen %zu", statusLen);
  char *match = strstr(str, statusStr);
  DEBUG("match %s", match);

  assert(match && "There is no tag in this string.");

  size_t idx = statusLen;
  size_t offset = idx;
  /* Moving past the tag line... */
  while (match[idx] != '\0') {

    retStr[idx - offset] = match[idx];
    ++idx;
  }
  /* Make the last 2 characters a space and a null term */
  retStr[idx - offset + 1] = ' ';
  retStr[idx - offset + 2] = '\0';
  DEBUG("-->FIN match str: %s", statusStr);

  strncpy(str, retStr, MAX_TODO_LEN - 1);
}

/* Function that takes a string without a status and then prepends :TODO: to it.
 */
char *handle_no_status(char *str) {

  DEBUG("--> There is no status for line: %s", str);

  /* Initialises the string (necessary) */
  char appendedStr[MAX_TODO_LEN] = {""};
  char *stat = status_enum_to_string(e_status_none);

  strncat(appendedStr, stat, strlen(stat));
  strncat(appendedStr, " ", 1);
  strncat(appendedStr, str, MAX_TODO_LEN - strlen(stat) - 2);

  strncpy(str, appendedStr, MAX_TODO_LEN - 1);

  DEBUG("Returning %s", str);
  return str;
}

/* TODO Perhaps implement this to be able to change a specific line in the file?
  Another idea is that we can just write to the file when we exit or add a new
  item / delete item etc. */
void change_todo_status_file_str(void) {
  /* void change_todo_status_file_str(Line_t *line, TODO_STATUS_e new_status) {
   */
  /* line = NULL; */
  /* new_status = 0; */

  FILE *file = fopen(todo_file_name, "rw");
  char currLine[MAX_TODO_LEN];

  while (fgets(currLine, MAX_TODO_LEN - 1, file) != NULL) {
  }
}

/* Will replace a string that contains a tag, with a checkbox instead.  */
void replace_tag_with_checkbox(char *str, TODO_STATUS_e status) {

  char boxedLine[MAX_TODO_LEN] = "";
  char *box = convert_status_to_box(status);
  size_t boxLen = strlen(box);
  size_t tagLen = strlen(status_enum_to_string(status));

  strncat(boxedLine, box, boxLen);
  DEBUG("Str right now: %s, length of box: %zu", boxedLine, boxLen);
  strncat(boxedLine, &str[tagLen], MAX_TODO_LEN - boxLen - 1);
  DEBUG("Str right now: %s", boxedLine);

  strncpy(str, boxedLine, MAX_TODO_LEN - 1);
  DEBUG("Str right now: %s", str);
}

/* Takes a Status enum and then returns a string literal that associates with it
 */
char *status_enum_to_string(TODO_STATUS_e status) {

  switch (status) {
  case e_status_complete:
    return ":DONE:";
  case e_status_incomplete:
    return ":TODO:";
  case e_status_none:
    return ":TODO:";
    break;
  }
  return ":TODO:";
}

/* Returns a checkbox for the specified status */
char *convert_status_to_box(TODO_STATUS_e status) {

  switch (status) {
  case e_status_complete:
    return "[x]";
  case e_status_incomplete:
    return "[ ]";
  case e_status_none:
    return "[ ]";
    break;
  }
  return "[ ]";
}

/* Takes a string and determines if the line is done or not */
TODO_STATUS_e parse_todo_status(char *str) {

  DEBUG("%s %s", "---> Parsing the todo status of line:", str);
  /* List of things to look for */
  const char *done = ":DONE:";
  const char *todo = ":TODO:";
  /* char *done = ":DONE:"; */

  if (strstr(str, done)) {
    return e_status_complete;
  } else if (strstr(str, todo)) {
    return e_status_incomplete;
  } else
    return e_status_none;

  assert(false && "Checkbox not found. Handle this.");

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
