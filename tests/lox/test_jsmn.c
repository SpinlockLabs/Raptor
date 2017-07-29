#include <assert.h>

#include <liblox/string.h>
#include <liblox/json/jsmn.h>
#include <liblox/memory.h>

int main() {
    jsmn_parser parser = {0};
    jsmn_init(&parser);

    const char* json = "{\"a\": \"b\", \"c\": 3000}";

    jsmn_token* tokens = NULL;

    int result = jsmn_parse(
        &parser,
        json,
        strlen(json),
        &tokens
    );

    assert(result == 5);

    free(tokens);
    return 0;
}
