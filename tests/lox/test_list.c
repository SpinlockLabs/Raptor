#include <assert.h>

#include <liblox/list.h>
#include <liblox/string.h>

int main() {
    list_t* list = list_create();
    list_add(list, "Hello World");

    assert(list->size == 1);
    assert(strcmp(list_get_value_at(list, 0), "Hello World") == 0);

    return 0;
}
