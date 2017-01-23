#include "tty.h"
#include <string.h>

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t) uc | (uint16_t) color << 8;
}

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
            vga_putentryat(' ', vga_color, x, y);
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

void vga_setcolor(uint8_t color) {
    vga_color = color;
}

void vga_putentryat(char c, uint8_t color, size_t x, size_t y) {
    const size_t index = y * VGA_WIDTH + x;
    vga_buffer[index] = vga_entry(c, color);
}

void vga_nextrow(void) {
    if (vga_row == VGA_HEIGHT - 1) {
        vga_scroll();
    } else {
        vga_row++;
    }
    vga_column = 0;
}

void vga_putchar(char c) {
    if (c == '\n') {
        vga_nextrow();
    } else {
        vga_putentryat(c, vga_color, vga_column, vga_row);
        if (++vga_column == VGA_WIDTH) {
            vga_column = 0;
            vga_nextrow();
        }
    }
}

void vga_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++ ) {
        vga_putchar(data[i]);
    }
}

void vga_writestring(const char* data) {
    vga_write(data, strlen(data));
}

