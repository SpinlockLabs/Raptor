/**
 * The Raptor OS string buffer library.
 */
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct strbuf {
    /**
     * Buffer size.
     */
    size_t size;

    /**
     * String null-termination index.
     */
    size_t term;

    struct {
        /**
         * Current cursor position.
         */
        size_t position;
    } cursor;

    char buffer[];
} strbuf_t;

strbuf_t* strbuf_create(size_t size);
void strbuf_init(strbuf_t* buf, size_t size);
void strbuf_destroy(strbuf_t* buf);

bool strbuf_can_move_to(strbuf_t* buf, size_t position);
bool strbuf_can_write_to(strbuf_t* buf, size_t position);

bool strbuf_move_left(strbuf_t* buf);
bool strbuf_move_right(strbuf_t* buf);
bool strbuf_move_begin(strbuf_t* buf);
bool strbuf_move_end(strbuf_t* buf);
bool strbuf_move_to(strbuf_t* buf, size_t position);
bool strbuf_backspace(strbuf_t* buf);

char strbuf_getc(strbuf_t* buf);
bool strbuf_putc(strbuf_t* buf, char c);
bool strbuf_puts(strbuf_t* buf, char* str);
bool strbuf_write(strbuf_t* buf, char* str, size_t len);
char* strbuf_read(strbuf_t* buf);
char* strbuf_copy(strbuf_t* buf);
void strbuf_clear(strbuf_t* buf);

bool strbuf_end(strbuf_t* buf);
