#include "../memory.h"
#include "jsmn.h"

/**
 * Allocates a fresh unused token from the token pool.
 */
static jsmn_token* jsmn_alloc_token(jsmn_parser* parser,
                                    jsmn_token* tokens, size_t num_tokens) {
    jsmn_token* tok;
    if (parser->next >= num_tokens) {
        return NULL;
    }
    tok = &tokens[parser->next++];
    tok->start = tok->end = -1;
    tok->size = 0;
    tok->parent = -1;
    return tok;
}

/**
 * Fills token type and boundaries.
 */
static void jsmn_fill_token(jsmn_token* token, jsmn_type_t type,
                            int start, int end) {
    token->type = type;
    token->start = start;
    token->end = end;
    token->size = 0;
}

/**
 * Fills next available token with JSON primitive.
 */
static int jsmn_parse_primitive(jsmn_parser* parser, const char* js,
                                size_t len, jsmn_token* tokens, size_t num_tokens) {
    jsmn_token* token;
    size_t start;

    start = parser->position;

    for (; parser->position < len && js[parser->position] != '\0'; parser->position++) {
        switch (js[parser->position]) {
            case ':':
                if (parser->flags.strict) {
                    break;
                }
            case '\t':
            case '\r':
            case '\n':
            case ' ':
            case ',':
            case ']':
            case '}':
                goto found;
            default:
                break;
        }

        if (js[parser->position] < 32 || js[parser->position] >= 127) {
            parser->position = start;
            return JSMN_ERROR_INVAL;
        }
    }

    if (parser->flags.strict) {
        /* In strict mode primitive must be followed by a comma/object/array */
        parser->position = start;
        return JSMN_ERROR_PART;
    }

    found:
    if (tokens == NULL) {
        parser->position--;
        return 0;
    }
    token = jsmn_alloc_token(parser, tokens, num_tokens);
    if (token == NULL) {
        parser->position = start;
        return JSMN_ERROR_NOMEM;
    }
    jsmn_fill_token(token, JSMN_PRIMITIVE, start, parser->position);
    token->parent = parser->root;
    parser->position--;
    return 0;
}

/**
 * Fills next token with JSON string.
 */
static int jsmn_parse_string(jsmn_parser* parser, const char* js,
                             size_t len, jsmn_token* tokens, size_t num_tokens) {
    jsmn_token* token;

    size_t start = parser->position;

    parser->position++;

    /* Skip starting quote */
    for (; parser->position < len && js[parser->position] != '\0'; parser->position++) {
        char c = js[parser->position];

        /* Quote: end of string */
        if (c == '\"') {
            if (tokens == NULL) {
                return 0;
            }
            token = jsmn_alloc_token(parser, tokens, num_tokens);
            if (token == NULL) {
                parser->position = start;
                return JSMN_ERROR_NOMEM;
            }
            jsmn_fill_token(token, JSMN_STRING, start + 1, parser->position);
            token->parent = parser->root;
            return 0;
        }

        /* Backslash: Quoted symbol expected */
        if (c == '\\' && parser->position + 1 < len) {
            int i;
            parser->position++;
            switch (js[parser->position]) {
                /* Allowed escaped symbols */
                case '\"':
                case '/' :
                case '\\' :
                case 'b' :
                case 'f' :
                case 'r' :
                case 'n'  :
                case 't' :
                    break;
                    /* Allows escaped symbol \uXXXX */
                case 'u':
                    parser->position++;
                    for (i = 0; i < 4 && parser->position < len && js[parser->position] != '\0'; i++) {
                        /* If it isn't a hex character we have an error */
                        if (!((js[parser->position] >= 48 && js[parser->position] <= 57) || /* 0-9 */
                              (js[parser->position] >= 65 && js[parser->position] <= 70) || /* A-F */
                              (js[parser->position] >= 97 && js[parser->position] <= 102))) { /* a-f */
                            parser->position = start;
                            return JSMN_ERROR_INVAL;
                        }
                        parser->position++;
                    }
                    parser->position--;
                    break;
                    /* Unexpected symbol */
                default:
                    parser->position = start;
                    return JSMN_ERROR_INVAL;
            }
        }
    }
    parser->position = start;
    return JSMN_ERROR_PART;
}

/**
 * Parse JSON string and fill tokens.
 */
int jsmn_parse_stage(jsmn_parser* parser, const char* js, size_t len,
                     jsmn_token* tokens, unsigned int num_tokens) {
    if (!parser->flags.initialized) {
        jsmn_init(parser);
    }

    int r;
    int i;
    jsmn_token* token;
    int count = parser->next;

    for (; parser->position < len && js[parser->position] != '\0'; parser->position++) {
        char c;
        jsmn_type_t type;

        c = js[parser->position];
        switch (c) {
            case '{':
            case '[':
                count++;
                if (tokens == NULL) {
                    break;
                }
                token = jsmn_alloc_token(parser, tokens, num_tokens);
                if (token == NULL) {
                    return JSMN_ERROR_NOMEM;
                }
                if (parser->root != -1) {
                    tokens[parser->root].size++;
                    token->parent = parser->root;
                }
                token->type = (c == '{' ? JSMN_OBJECT : JSMN_ARRAY);
                token->start = parser->position;
                parser->root = parser->next - 1;
                break;
            case '}':
            case ']':
                if (tokens == NULL) {
                    break;
                }
                type = (c == '}' ? JSMN_OBJECT : JSMN_ARRAY);
                if (parser->next < 1) {
                    return JSMN_ERROR_INVAL;
                }
                token = &tokens[parser->next - 1];
                for (;;) {
                    if (token->start != -1 && token->end == -1) {
                        if (token->type != type) {
                            return JSMN_ERROR_INVAL;
                        }
                        token->end = parser->position + 1;
                        parser->root = token->parent;
                        break;
                    }
                    if (token->parent == -1) {
                        if (token->type != type || parser->root == -1) {
                            return JSMN_ERROR_INVAL;
                        }
                        break;
                    }
                    token = &tokens[token->parent];
                }
                for (i = parser->next - 1; i >= 0; i--) {
                    token = &tokens[i];
                    if (token->start != -1 && token->end == -1) {
                        if (token->type != type) {
                            return JSMN_ERROR_INVAL;
                        }
                        parser->root = -1;
                        token->end = parser->position + 1;
                        break;
                    }
                }
                /* Error if unmatched closing bracket */
                if (i == -1) {
                    return JSMN_ERROR_INVAL;
                }
                for (; i >= 0; i--) {
                    token = &tokens[i];
                    if (token->start != -1 && token->end == -1) {
                        parser->root = i;
                        break;
                    }
                }
                break;
            case '\"':
                r = jsmn_parse_string(parser, js, len, tokens, num_tokens);
                if (r < 0) {
                    return r;
                }
                count++;
                if (parser->root != -1 && tokens != NULL) {
                    tokens[parser->root].size++;
                }
                break;
            case '\t' :
            case '\r' :
            case '\n' :
            case ' ':
                break;
            case ':':
                parser->root = parser->next - 1;
                break;
            case ',':
                if (tokens != NULL && parser->root != -1 &&
                    tokens[parser->root].type != JSMN_ARRAY &&
                    tokens[parser->root].type != JSMN_OBJECT) {
                    parser->root = tokens[parser->root].parent;
                }
                break;
                /* In strict mode primitives are: numbers and booleans */
            case '-':
            case '0':
            case '1' :
            case '2':
            case '3' :
            case '4':
            case '5':
            case '6':
            case '7' :
            case '8':
            case '9':
            case 't':
            case 'f':
            case 'n' :
                if (parser->flags.strict) {
                    /* And they must not be keys of the object */
                    if (tokens != NULL && parser->root != -1) {
                        jsmn_token* t = &tokens[parser->root];
                        if (t->type == JSMN_OBJECT ||
                            (t->type == JSMN_STRING && t->size != 0)) {
                            return JSMN_ERROR_INVAL;
                        }
                    }
                }
                /* In non-strict mode every unquoted value is a primitive */
            default:
                if (parser->flags.strict) {
                    return JSMN_ERROR_INVAL;
                }

                r = jsmn_parse_primitive(parser, js, len, tokens, num_tokens);
                if (r < 0) {
                    return r;
                }
                count++;
                if (parser->root != -1 && tokens != NULL) {
                    tokens[parser->root].size++;
                }
                break;
        }
    }

    if (tokens != NULL) {
        for (i = parser->next - 1; i >= 0; i--) {
            /* Unmatched opened object or array */
            if (tokens[i].start != -1 && tokens[i].end == -1) {
                return JSMN_ERROR_PART;
            }
        }
    }

    return count;
}

int jsmn_parse(jsmn_parser* parser, const char* js, size_t len,
               jsmn_token** tokens) {
    if (!parser->flags.initialized) {
        jsmn_init(parser);
    }

    unsigned int pool_count = 10;
    int state = 1;
    int ret = 0;

    jsmn_token* pool = zalloc(sizeof(jsmn_token) * pool_count);
    while (state != 0) {
        int result = jsmn_parse_stage(
          parser,
          js,
          len,
          pool,
          pool_count
        );

        if (result == JSMN_ERROR_NOMEM) {
            state = 1;
            pool_count += 10;
            pool = realloc(pool, sizeof(jsmn_token) * pool_count);
        } else if (result == JSMN_ERROR_PART) {
            state = 0;
            ret = JSMN_ERROR_PART;
        } else if (result == JSMN_ERROR_INVAL) {
            state = 0;
            ret = JSMN_ERROR_INVAL;
        } else {
            state = 0;
            ret = result;
        }
    }

    if (ret >= 0) {
        int tkcnt = ret;
        if (tkcnt == 0) {
            tkcnt = 1;
        }

        pool = realloc(pool, sizeof(jsmn_token) * tkcnt);
        *tokens = pool;
    }

    return ret;
}

/**
 * Creates a new parser based over a given buffer with an array of tokens
 * available.
 */
void jsmn_init(jsmn_parser* parser) {
    parser->position = 0;
    parser->next = 0;
    parser->root = -1;
    parser->flags.strict = true;
    parser->flags.initialized = true;
}
