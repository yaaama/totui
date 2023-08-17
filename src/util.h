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

#endif // UTIL_H_
