#ifndef UTIL_H_
#define UTIL_H_

/******************************/
/* /\* Imports and Macros: *\/ */
/******************************/

/* My headers */
#include "ui.h"
/* Libraries */
#include <assert.h>
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

#define LOG_FILE "log.txt"

/* Used to print debug information to the file specified in LOG_FILE */
#define DEBUG(fmt, ...)                                                        \
  do {                                                                         \
    FILE *file = fopen(LOG_FILE, "a");                                         \
    if (file != NULL) {                                                        \
      fprintf(file, "%s\t%s: " fmt "\n", get_local_time(), __func__,           \
              ##__VA_ARGS__);                                                  \
      fclose(file);                                                            \
    }                                                                          \
  } while (0)

/* ------------------------------------------------------------------------ */

/****************************/
/* /\* Global Variables *\/ */
/****************************/

extern char *todo_file_name; /* Name of todo file */

/*********************/
/* /\* Functions *\/ */
/*********************/

/* Necessary for debugging */
const char *get_local_time(void);

/* File handling functions */
void setup_log_file(char *filename);
LineList_t *load_todo_file(char *filename); /* Loads up the todo txt file */
void append_to_file(char *filename,
                    char *str); /* Appends str to the todo file */

void print_all_todo_items(
    LineList_t
        *line_list); /* Prints all of the todo items stored in the LineList */
TODO_STATUS_e parse_todo_status(char *str);
char *status_enum_to_string(TODO_STATUS_e status);
char *convert_status_to_box(TODO_STATUS_e status);
void cut_tag_from_line_string(char *str, TODO_STATUS_e status);

#endif // UTIL_H_
