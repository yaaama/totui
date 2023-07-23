#include "ui.h"
#include <ncurses.h>
#include <stdlib.h>

                                /* External var */
size_t initial_file_lines_count;

/***************/
/* Definitions */
/***************/
/* void ui_init_attribute(void); */
/* void ui_init_colors(void); */
/* void ui_highlight_line(Line_t *line); */
/* void ui_refresh_screen(void); */

/**************************/
/* /\* Initialisation *\/ */
/**************************/
/* void ui_init_helpbar(void); */
/* void ui_init_line(void); */


void ui_init_colours(void) {

  start_color();

  init_pair(1, COLOR_RED, COLOR_RED);
  init_pair(2, COLOR_GREEN, COLOR_BLACK);
  init_pair(3, COLOR_BLUE, COLOR_BLACK);
  init_pair(4, COLOR_CYAN, COLOR_BLACK);



}

                                /* This will print the text all nicely on the window of each line */
void ui_init_lines(Line_t *line) {

  WINDOW *currW = NULL;
  currW = line->line;
                                /* Make a border
                                  Colourise the output perhaps
                                  */



}

Screen_t *ui_init_screen(Line_t **lines) {

  Screen_t *screen = malloc(sizeof(Screen_t));
  screen->lines_total = 0;
  /* TODO Initialise echo bar and help bar */

  for (size_t i = 0; i < initial_file_lines_count; i++) {
    size_t currRow = i + 1;

    screen->lines[i] = newwin(1, COLS, currRow, 0);
    screen->lines_total++;
  }
return screen;
}


void ui_init(Line_t **lines) {

  initscr();
  clear();
  nonl(); /* Controls where ENTER key is drawn onto page */
  noecho();
  refresh();
  /* curs_set(0); */

  Screen_t *scrn = ui_init_screen(lines);

}
