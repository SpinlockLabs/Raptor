#pragma once

#include <kernel/tty.h>

#include <stddef.h>
#include <stdint.h>

extern tty_t *vga_pty;

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

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

size_t vga_row;
size_t vga_column;
uint8_t vga_color;
uint16_t* vga_buffer;

void vga_init(void);
void vga_clear(void);
void vga_scroll(void);
void vga_set_color(uint8_t);
void vga_set_at(char, uint8_t, size_t, size_t);
void vga_cursor(size_t, size_t);
void vga_nextrow(void);
void vga_putchar(char);
void vga_write(const char*, size_t);
void vga_write_string(const char*);
