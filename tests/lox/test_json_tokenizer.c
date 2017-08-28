#include <liblox/string.h>
#include <liblox/json/jsmn.h>
#include <liblox/memory.h>

#include "helper/test.h"

int main() {
    test_start("JSMN Tokenizer");

    jsmn_parser parser;
    jsmn_init(&parser);

    const char* json = "{\"a\": \"b\", \"c\": 3000}";

    jsmn_token* tokens = NULL;

    int result = jsmn_parse(
        &parser,
        json,
        strlen(json),
        &tokens
    );

    test_assert(
        result == 5,
        "Token count should be five."
    );

    test_assert(
        tokens[0].type == JSMN_OBJECT,
        "First token is not an object."
    );

    test_assert(
        tokens[1].type == JSMN_STRING,
        "Second token is not a string."
    );

    test_assert(
        tokens[2].type == JSMN_STRING,
        "Third token is not a string."
    );

    test_assert(
        tokens[3].type == JSMN_STRING,
        "Fourth token is not a string."
    );

    test_assert(
        tokens[4].type == JSMN_PRIMITIVE,
        "Fifth token is not a primitive."
    );

    free(tokens);
    test_end();

    return 0;
}
