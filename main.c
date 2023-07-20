#include "item.h"
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>

<<<<<<< HEAD
<<<<<<< Updated upstream
int main(void) {
=======
void test_todo_create(void) {
>>>>>>> fe37a6f (changed totui.c to main.c, can change todo status)
=======
void test_todo_create(void) {
=======
int main(void) {
>>>>>>> b964d15 (changed totui.c to main.c, can change todo status)
>>>>>>> Stashed changes

  todo_item item0 = {"Random todo thing", COMPLETE};
  todo_item item1 = {"Random todo thing2", COMPLETE};
  todo_item item2 = {"Random todo thing3", COMPLETE};
  change_item_status(&item2, PROGRESSING);
  todo_item item3 = {"Random todo thing4", COMPLETE};
  todo_item item4 = {"Random todo thing5", COMPLETE};

  todo_item items[5] = {item0, item1, item2, item3, item4};

  for (int i = 0; i < 5; i++) {
<<<<<<< HEAD
<<<<<<< Updated upstream
    printf("Todo:\t%s\t|\t\tStatus? %s\n",
           items[i].heading, status_string(items[i].status));
  }

  return 0;
}
=======
=======
>>>>>>> Stashed changes
    printf("Todo:\t%s\t|\t\tStatus? %s\n", items[i].heading,
           status_string(items[i].status));
  }
}

int main(void) { return 0; }
<<<<<<< Updated upstream
>>>>>>> fe37a6f (changed totui.c to main.c, can change todo status)
=======
=======
    printf("Todo:\t%s\t|\t\tStatus? %s\n",
           items[i].heading, status_string(items[i].status));
  }

  return 0;
}
>>>>>>> b964d15 (changed totui.c to main.c, can change todo status)
>>>>>>> Stashed changes
