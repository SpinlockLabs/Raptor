#pragma once

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * JSON type identifier. Basic types are:
 * 	o Object
 * 	o Array
 * 	o String
 * 	o Other primitive: number, boolean (true/false) or null
 */
typedef enum {
    JSMN_UNDEFINED = 0,
    JSMN_OBJECT = 1,
    JSMN_ARRAY = 2,
    JSMN_STRING = 3,
    JSMN_PRIMITIVE = 4
} jsmn_type_t;

typedef enum jsmn_error {
    /* Not enough tokens were provided */
    JSMN_ERROR_NOMEM = -1,
    /* Invalid character inside JSON string */
    JSMN_ERROR_INVAL = -2,
    /* The string is not a full JSON packet, more bytes expected */
    JSMN_ERROR_PART = -3
} jsmn_error_t;

/**
 * JSON token description.
 * type		type (object, array, string etc.)
 * start	start position in JSON data string
 * end		end position in JSON data string
 */
typedef struct {
    jsmn_type_t type;
    int start;
    int end;
    int size;
    int parent;
} jsmn_token;

/**
 * JSON parser. Contains an array of token blocks available. Also stores
 * the string being parsed now and current position in that string
 */
typedef struct {
    struct {
        bool initialized;
        bool strict;
    } flags;
    size_t position; /* offset in the JSON string */
    size_t next; /* next token to allocate */
    int root; /* superior token node, e.g parent object or array */
} jsmn_parser;

/**
 * Create JSON parser over an array of tokens
 */
void jsmn_init(jsmn_parser* parser);

/**
 * Run JSON parser. It parses a JSON data string into and array of tokens, each describing
 * a single JSON object.
 */
int jsmn_parse_stage(jsmn_parser* parser, const char* js, size_t len,
                     jsmn_token* tokens, unsigned int num_tokens);

/**
 * Run JSON parser. It parses a JSON data string into and array of tokens, each describing
 * a single JSON object.
 */
int jsmn_parse(jsmn_parser* parser, const char* js, size_t len,
               jsmn_token** tokens);


#ifdef __cplusplus
}
#endif
