#include <liblox/string.h>
#include <liblox/json/jsmn.h>
#include <liblox/io.h>
#include <liblox/memory.h>

int main() {
    jsmn_parser parser = {0};
    jsmn_init(&parser);

    const char* json = "{\"a\": 2}";

    jsmn_token* tokens = NULL;

    int result = jsmn_parse(
        &parser,
        json,
        strlen(json),
        &tokens
    );

    printf("%s\nresult: %d\n", json, result);

    free(tokens);
    return 0;
}
