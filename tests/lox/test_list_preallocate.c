#include <liblox/list.h>
#include <liblox/string.h>

#include "helper/test.h"

static const uint count = 1000000;

int main() {
    test_start("Preallocated List");
    list_t* list = list_pcreate(count);
    list->free_values = false;

    list_for_each(node, list) {
        node->value = "Hello World";
    }

    test_assert(
        list->size == count,
        "List size is not equal to what we requested."
    );

    list_for_each(node, list) {
        test_assert(
            strcmp(node->value, "Hello World") == 0,
            "Stored value is not equal to the value we got back."
        );
    }

    list_for_each(node, list) {
        list_remove(node);
    }

    list_free(list);
    test_end();
    return 0;
}
