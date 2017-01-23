#pragma once
#include <stdint.h>
#include <stddef.h>

enum vga_color {
    VGA_COLOR_BLACK,
    VGA_COLOR_BLUE,
    VGA_COLOR_GREEN,
    VGA_COLOR_CYAN,
    VGA_COLOR_RED,
    VGA_COLOR_MAGENTA,
    VGA_COLOR_BROWN,
    VGA_COLOR_LIGHT_GREY,
    VGA_COLOR_DARK_GREY,
    VGA_COLOR_LIGHT_BLUE,
    VGA_COLOR_LIGHT_GREEN,
    VGA_COLOR_LIGHT_CYAN,
    VGA_COLOR_LIGHT_RED,
    VGA_COLOR_LIGHT_MAGENTA,
    VGA_COLOR_LIGHT_BROWN,
    VGA_COLOR_WHITE
};

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;

size_t vga_row;
size_t vga_column;
uint8_t vga_color;
uint16_t* vga_buffer;

void vga_init(void);
void vga_clear(void);
void vga_scroll(void);
void vga_setcolor(uint8_t);
void vga_putentryat(char, uint8_t, size_t, size_t);
void vga_nextrow(void);
void vga_putchar(char);
void vga_write(const char*, size_t);
void vga_writestring(const char*);

