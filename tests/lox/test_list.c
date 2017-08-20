#include <assert.h>

#include <liblox/list.h>
#include <liblox/string.h>

static const uint count = 1000000;

int main() {
    list_t* list = list_pcreate(count);

    list_for_each(node, list) {
        node->value = "Hello World";
    }

    assert(list->size == count);

    list_for_each(node, list) {
        assert(strcmp(node->value, "Hello World") == 0);
    }

    return 0;
}
