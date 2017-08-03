#include <liblox/string.h>
#include <liblox/io.h>

#include "io.h"
#include "vga.h"

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t) uc | (uint16_t) color << 8;
}

tty_t* vga_pty = NULL;

void vga_init(void) {
    vga_row = 0;
    vga_column = 0;
    vga_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_buffer = (uint16_t*) 0xB8000;

    // Previous code may have left the terminal dirty.
    vga_clear();
}

void vga_clear(void) {
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            vga_set_at(' ', vga_color, x, y);
        }
    }
}

/**
 * Scroll the terminal by one line.
 */
void vga_scroll(void) {
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[y * VGA_WIDTH + x] = vga_buffer[(y + 1) * VGA_WIDTH + x];
        }
    }
}

void vga_set_color(uint8_t color) {
    vga_color = color;
}

void vga_set_at(char c, uint8_t color, size_t x, size_t y) {
    size_t index = y * VGA_WIDTH + x;
    vga_buffer[index] = vga_entry((unsigned char) c, color);
}

void vga_set_entry_at(uint16_t value, size_t x, size_t y) {
    size_t index = y * VGA_WIDTH + x;
    vga_buffer[index] = value;
}

uint16_t vga_read_entry_at(size_t col, size_t row) {
    size_t index = row * VGA_WIDTH + col;
    return vga_buffer[index];
}

void vga_cursor(size_t col, size_t row) {
    vga_column = col;
    vga_row = row;

    uint16_t pos = (uint16_t) ((row * VGA_WIDTH) + col);

    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t) (pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
}

void vga_get_cursor(size_t* col, size_t* row) {
    uint16_t offset;
    outb(0x3D4, 14);
    offset = inb(0x3D5) << 8;
    outb(0x3D4, 15);
    offset |= inb(0x3D5);

    *row = (size_t) (offset / VGA_WIDTH);
    *col = (size_t) (offset % VGA_WIDTH);
}

void vga_nextrow(void) {
    if (vga_row == VGA_HEIGHT - 1) {
        vga_scroll();
    } else {
        vga_row++;
    }
    vga_column = 0;
}

void vga_putchar_direct(char c, bool user) {
    if (c == '\n') {
        vga_nextrow();
    } else if (c == '\b') {
        if (!user) {
            uint16_t after[VGA_WIDTH] = {0};
            for (size_t i = vga_column; i < VGA_WIDTH; i++) {
                after[i] = vga_read_entry_at(i, vga_row);
            }

            for (size_t i = vga_column - 1; i < VGA_WIDTH - 1; i++) {
                vga_set_entry_at(after[i + 1], i, vga_row);
            }
            vga_set_at(' ', vga_color, VGA_WIDTH - 1, vga_row);

            vga_column -= 1;
        }
    } else {
        vga_set_at(c, vga_color, vga_column, vga_row);
        if (++vga_column == VGA_WIDTH) {
            vga_column = 0;
            vga_nextrow();
        }
    }
    vga_cursor(vga_column, vga_row);
}

void vga_putchar(char c) {
    vga_putchar_direct(c, false);
}

static void vga_handle_ansi(const char* data, size_t size) {
    unused(size);

    size_t col = vga_column;
    size_t row = vga_row;

    char c = data[2];

    if (c == 'D') {
        if (vga_column != 0) {
            vga_cursor(col - 1, row);
        }
        return;
    }

    if (c == 'C') {
        if (vga_column < VGA_WIDTH) {
            vga_cursor(col + 1, row);
        }
        return;
    }
}

void vga_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++ ) {
        if (i == 0 && data[i] == '\x1b' && size >= 3) {
            vga_handle_ansi(data, size);
            return;
        }

        vga_putchar(data[i]);
    }
}

void vga_write_string(const char* data) {
    if (data == NULL) {
        return;
    }
    vga_write(data, strlen(data));
}
