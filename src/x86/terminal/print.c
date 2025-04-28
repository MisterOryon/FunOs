//
// Created by oryon on 4/25/25.
//

#include "print.h"

#include <io.h>
#include <stddef.h>
#include <stdint.h>

static uint16_t* g_video_mem = NULL;
static unsigned g_cursor_row = 0;
static unsigned g_cursor_column = 0;
static const vga_color_attributes_t DEFAULT_TEXT_COLORS = {COLOR_WHITE, COLOR_BLACK};

/**
 * Calculates the length of a null-terminated string.
 *
 * @param str A pointer to the null-terminated string whose length is to be calculated.
 * @return The length of the string, excluding the null-terminator.
 */
static size_t string_length(const char* str)
{
    size_t len = 0;
    while (str[len]) len++;
    return len;
}

/**
 * Creates a VGA character with specified attributes and character value.
 *
 * @param c The character to be encoded.
 * @param color A structure containing the foreground and background color attributes for the character.
 * @return A 16-bit encoded VGA character combining the character value and color attributes.
 */
static uint16_t vga_create_char(const char c, const vga_color_attributes_t color)
{
    const uint16_t attr = (color.background << VGA_BACKCOLOR_OFFSET) | (color.foreground & VGA_COLOR_MASK);
    return attr << VGA_COLOR_ATTRIBUTE_SHIFT | c;
}

/**
 * Writes a character with associated attributes to a specific position
 * in the VGA text buffer.
 *
 * @param x The horizontal coordinate of the character position. Must be within the valid range [0, VGA_WIDTH).
 * @param y The vertical coordinate of the character position. Must be within the valid range [0, VGA_HEIGHT).
 * @param c The character and its attributes (foreground and background color) packed into a 16-bit value.
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
 * Enables the hardware text-mode cursor by setting its starting and ending scanline positions.
 *
 * @param cursor_start The starting scanline of the cursor.
 * @param cursor_end The ending scanline of the cursor.
 */
static void vga_enable_cursor(const uint8_t cursor_start, const uint8_t cursor_end)
{
    io_outb(0x3D4, 0x0A);
    io_outb(0x3D5, (io_inb(0x3D5) & 0xC0) | cursor_start);

    io_outb(0x3D4, 0x0B);
    io_outb(0x3D5, (io_inb(0x3D5) & 0xE0) | cursor_end);
}

/**
 * Updates the cursor position on the VGA text-mode display.
 */
static void vga_update_cursor_position(void)
{
    const uint16_t pos = (g_cursor_row * VGA_WIDTH) + g_cursor_column;

    if (pos >= VGA_WIDTH * VGA_HEIGHT)
        return;

    io_outb(0x3D4, 0x0F);
    io_outb(0x3D5, pos & 0xFF);
    io_outb(0x3D4, 0x0E);
    io_outb(0x3D5, (pos >> 8) & 0xFF);
}

/**
 * Clears a single line on the display by writing blank characters with the
 * default background and foreground colors.
 *
 * @param y The y-coordinate of the line to be cleared.
 * The value must be within the vertical bounds of the display.
 */
static void display_clear_line(const unsigned y)
{
    const uint16_t black = vga_create_char(0x0, DEFAULT_TEXT_COLORS);
    for (int x = 0; x < VGA_WIDTH; x++)
        vga_write_char_at(x, y, black);
}

/**
 * Scrolls the contents of the display up by one line.
 *
 * Copies all lines of the video memory buffer up one position and erases the
 * bottom-most line by filling it with blank spaces.
 * This effectively shifts all displayed content up one row, making space for
 * new content at the bottom.
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
 * Handles line wrapping for the cursor in a VGA text mode display.
 *
 * Resets the cursor's column position to 0 when it reaches the end of a line.
 * If the cursor's row position is at the last row of the screen, the display is scrolled up.
 * Otherwise, the cursor moves to the next row.
 * The new line where the cursor moves is cleared to prepare for further text.
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
 * Writes a single character with specified color attributes to the display.
 * Handles line wrapping and newline characters appropriately.
 *
 * @param c The character to be written to the display.
 * @param color The VGA color attributes to apply to the character, including foreground and background colors.
 */
static void display_put_char(const char c, const vga_color_attributes_t color)
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
 * Sets the cursor position on the display.
 */
int display_set_cursor_position(const unsigned x, const unsigned y)
{
    if (x >= VGA_WIDTH || y >= VGA_HEIGHT)
        return -1;

    g_cursor_column = x;
    g_cursor_row = y;
    vga_update_cursor_position();
    return 0;
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
    vga_enable_cursor(14, 15);
    vga_update_cursor_position();
}

/**
 * Print a string to the terminal
 */
void console_write_string(const char* str)
{
    const size_t len = string_length(str);
    for (size_t i = 0; i < len; i++)
        display_put_char(str[i], DEFAULT_TEXT_COLORS);
    vga_update_cursor_position();
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
        vga_update_cursor_position();
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
    vga_update_cursor_position();
}

/**
 * Writes a signed integer to the console.
 */
void console_write_int(const int value)
{
    if (value & INT_SIGN_BIT_MASK)
    {
        display_put_char('-', DEFAULT_TEXT_COLORS);
        console_write_uint((~(unsigned)value) + 1);
        return;
    }

    console_write_uint(value);
}
