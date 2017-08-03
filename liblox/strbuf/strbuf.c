#include "strbuf.h"

#include "../memory.h"
#include "../string.h"

strbuf_t* strbuf_create(size_t size) {
    strbuf_t* buf = zalloc(sizeof(strbuf_t) + size + 1);
    strbuf_init(buf, size);
    return buf;
}

void strbuf_init(strbuf_t* buf, size_t size) {
    buf->size = size;
    buf->term = 0;
    buf->cursor.position = 0;
}

bool strbuf_can_move_to(strbuf_t* buf, size_t position) {
    if (position > buf->term) {
        return false;
    }

    return true;
}

bool strbuf_can_write_to(strbuf_t* buf, size_t position) {
    if (position >= buf->size) {
        return false;
    }

    return true;
}

bool strbuf_move_to(strbuf_t* buf, size_t position) {
    if (!strbuf_can_move_to(buf, position)) {
        return false;
    }

    buf->cursor.position = position;
    return true;
}

bool strbuf_move_left(strbuf_t* buf) {
    if (buf->cursor.position == 0) {
        return false;
    }

    return strbuf_move_to(buf, buf->cursor.position - 1);
}

bool strbuf_move_right(strbuf_t* buf) {
    return strbuf_move_to(buf, buf->cursor.position + 1);
}

bool strbuf_move_begin(strbuf_t* buf) {
    return strbuf_move_to(buf, 0);
}

bool strbuf_move_end(strbuf_t* buf) {
    return strbuf_move_to(buf, buf->term);
}

bool strbuf_putc(strbuf_t* buf, char c) {
    if (!strbuf_can_write_to(buf, buf->cursor.position)) {
        return false;
    }

    buf->buffer[buf->cursor.position] = c;

    if (buf->cursor.position == buf->term) {
        buf->term++;
        buf->buffer[buf->term] = '\0';
    }

    buf->cursor.position++;

    return true;
}

bool strbuf_backspace(strbuf_t* buf) {
    if (!strbuf_can_move_to(buf, buf->cursor.position - 1)) {
        return false;
    }

    if (buf->cursor.position == buf->term) {
        buf->term--;
        buf->cursor.position--;
        buf->buffer[buf->term] = '\0';
        return true;
    }

    strdelcat(buf->buffer, buf->cursor.position - 1);
    buf->term--;
    buf->cursor.position--;
    return true;
}

bool strbuf_write(strbuf_t* buf, char* str, size_t len) {
    size_t end = buf->cursor.position + len;
    if (!strbuf_can_write_to(buf, end)) {
        return false;
    }

    memcpy(&buf->buffer[buf->cursor.position], str, len);
    buf->term = end;
    buf->buffer[buf->term] = '\0';
    return strbuf_move_to(buf, end);
}

bool strbuf_puts(strbuf_t* buf, char* str) {
    size_t len = strlen(str);
    return strbuf_write(buf, str, len);
}

char* strbuf_read(strbuf_t* buf) {
    return buf->buffer;
}

char* strbuf_copy(strbuf_t* buf) {
    return strdup(buf->buffer);
}

bool strbuf_end(strbuf_t* buf) {
    return buf->cursor.position == buf->term;
}

void strbuf_clear(strbuf_t* buf) {
    memset(buf->buffer, 0, buf->size + 1);
    buf->cursor.position = 0;
    buf->term = 0;
}

char strbuf_getc(strbuf_t* buf) {
    if (buf->cursor.position == 0) {
        return '\0';
    }
    return buf->buffer[buf->cursor.position - 1];
}

void strbuf_destroy(strbuf_t* buf) {
    free(buf);
}
