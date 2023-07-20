#include "tests.h"
#include "item.h"

void test_todo_create(void) {

  todo_item item0 = {"Random todo thing", COMPLETE};
  todo_item item1 = {"Random todo thing2", COMPLETE};
  todo_item item2 = {"Random todo thing3", COMPLETE};
  change_item_status(&item2, PROGRESSING);
  todo_item item3 = {"Random todo thing4", COMPLETE};
  todo_item item4 = {"Random todo thing5", COMPLETE};

  todo_item items[5] = {item0, item1, item2, item3, item4};

  for (int i = 0; i < 5; i++) {
    printf("Todo:\t%s\t|\t\tStatus? %s\n", items[i].heading,
           status_string(items[i].status));
  }
}
