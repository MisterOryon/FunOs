//
// Created by oryon on 4/25/25.
//

#ifndef PRINT_H
#define PRINT_H

/* VGA Display Constants */
#define VGA_MEMORY_ADDRESS 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

#define VGA_COLOR_MASK 0x0F
#define VGA_BACKCOLOR_OFFSET 4
#define VGA_COLOR_ATTRIBUTE_SHIFT 8

#define CHAR_NEWLINE '\n'


/* VGA Color Definitions */
typedef enum : unsigned char
{
    COLOR_BLACK = 0,
    COLOR_BLUE = 1,
    COLOR_GREEN = 2,
    COLOR_CYAN = 3,
    COLOR_RED = 4,
    COLOR_MAGENTA = 5,
    COLOR_BROWN = 6,
    COLOR_LIGHT_GRAY = 7,

    COLOR_DARK_GRAY = 8,
    COLOR_LIGHT_BLUE = 9,
    COLOR_LIGHT_GREEN = 10,
    COLOR_LIGHT_CYAN = 11,
    COLOR_LIGHT_RED = 12,
    COLOR_LIGHT_MAGENTA = 13,
    COLOR_YELLOW = 14,
    COLOR_WHITE = 15
} vga_color_t;

/* Color pair structure for foreground and background */
typedef struct
{
    vga_color_t foreground; // Represents the foreground color attribute for VGA text mode.
    vga_color_t background; // Represents the background color attribute for VGA text mode.
} vga_color_attributes_t;

/**
 * @brief Clears the terminal screen
 */
void display_clear(void);

/**
 * @brief Updates the cursor position on the screen.
 *
 * @param x The horizontal position of the cursor in character cells.
 * @param y The vertical position of the cursor in character cells.
 */
void display_set_cursor_position(const unsigned x, const unsigned y);

/**
 * @brief Initializes the terminal for printing
 */
void display_initialize(void);

/**
 * @brief Prints a string to the terminal
 * @param str The string to print
 */
void console_write_string(const char* str);

/**
 * @brief Prints an unsigned integer to the terminal
 * @param value The value to print
 */
void console_write_uint(unsigned value);

#endif //PRINT_H
