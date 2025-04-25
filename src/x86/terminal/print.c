//
// Created by oryon on 4/25/25.
//

#include "print.h"

#include <stddef.h>
#include <stdint.h>

static uint16_t* g_video_mem = NULL;
static unsigned g_cursor_row = 0;
static unsigned g_cursor_column = 0;
static const VgaColorAttributes DEFAULT_TEXT_COLORS = {COLOR_WHITE, COLOR_BLACK};
static const VgaColorAttributes CLEAR_COLORS = {COLOR_BLACK, COLOR_BLACK};

/**
 * Calculate string length
 */
static size_t string_length(const char* str)
{
    size_t len = 0;
    while (str[len]) len++;
    return len;
}

/**
 * Create a VGA character with the specified color attributes
 */
static uint16_t vga_create_char(const char c, const VgaColorAttributes color)
{
    const uint16_t attr = (color.background << VGA_BACKCOLOR_OFFSET) | (color.foreground & VGA_COLOR_MASK);
    return attr << VGA_COLOR_ATTRIBUTE_SHIFT | c;
}

/**
 * Put a character at the specified position
 */
static void vga_write_char_at(const unsigned x, const unsigned y, const uint16_t c)
{
    if (g_video_mem == NULL)
        return;
    if (x >= VGA_WIDTH || y >= VGA_HEIGHT)
        return;
    g_video_mem[y * VGA_WIDTH + x] = c;
}

/**
 * Clear a specific line on the terminal
 */
static void display_clear_line(const unsigned y)
{
    const uint16_t black = vga_create_char(' ', CLEAR_COLORS);
    for (int x = 0; x < VGA_WIDTH; x++)
        vga_write_char_at(x, y, black);
}

/**
 * Scroll the terminal up by one line
 */
static void display_scroll_up(void)
{
    const uint16_t* src = NULL;
    uint16_t* dest = NULL;

    for (int y = 0; y < VGA_HEIGHT - 1; y++)
    {
        src = &g_video_mem[(y + 1) * VGA_WIDTH];
        dest = &g_video_mem[y * VGA_WIDTH];

        for (int x = 0; x < VGA_WIDTH; x++)
            dest[x] = src[x];
    }
    display_clear_line(VGA_HEIGHT - 1);
}

/**
 * Handle terminal line overflow
 */
static void cursor_handle_line_wrap(void)
{
    g_cursor_column = 0;

    if (g_cursor_row >= VGA_HEIGHT - 1)
        display_scroll_up();
    else
        g_cursor_row++;

    display_clear_line(g_cursor_row);
}

/**
 * Write a character to terminal with a specified color
 */
static void display_put_char(const char c, const VgaColorAttributes color)
{
    const uint16_t char_code = vga_create_char(c, color);
    if (c == CHAR_NEWLINE)
    {
        cursor_handle_line_wrap();
        return;
    }

    vga_write_char_at(g_cursor_column, g_cursor_row, char_code);
    if (++g_cursor_column >= VGA_WIDTH)
        cursor_handle_line_wrap();
}

/**
 * Clear the entire terminal
 */
void display_clear(void)
{
    for (int y = 0; y < VGA_HEIGHT; y++)
        display_clear_line(y);
}

/**
 * Initialize the terminal
 */
void display_initialize(void)
{
    g_video_mem = (uint16_t*)VGA_MEMORY_ADDRESS;
    g_cursor_row = 0;
    g_cursor_column = 0;
    display_clear();
}

/**
 * Print a string to the terminal
 */
void console_write_string(const char* str)
{
    const size_t len = string_length(str);
    for (size_t i = 0; i < len; i++)
        display_put_char(str[i], DEFAULT_TEXT_COLORS);
}

/**
 * Print an unsigned integer to the terminal
 */
void console_write_uint(const unsigned value)
{
    char buffer[20];
    int pos = 0;
    unsigned temp = value;

    if (value == 0)
    {
        display_put_char('0', DEFAULT_TEXT_COLORS);
        return;
    }

    // Build the number backwards.
    while (temp > 0)
    {
        buffer[pos++] = (char)('0' + (temp % 10));
        temp /= 10;
    }

    // Display the numbers in the correct order
    while (pos > 0)
        display_put_char(buffer[--pos], DEFAULT_TEXT_COLORS);
}

