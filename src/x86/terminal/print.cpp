//
// Created by oryon on 4/25/25.
//

#include <stddef.h>
#include <stdint.h>

#include "print.h"
#include "io.h"

using namespace funos::console;

static uint16_t* gVideoMemPtr = nullptr;
static unsigned gCursorRowPos = 0;
static unsigned gCursorColumnPos = 0;
static constexpr vga_color_attributes_t DEFAULT_TEXT_COLORS = {COLOR_WHITE, COLOR_BLACK};

/**
 * Calculates the length of a null-terminated string.
 *
 * @param str A pointer to the null-terminated string whose length is to be calculated.
 * @return The length of the string, excluding the null-terminator.
 */
static size_t stringLength(const char* str)
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
static uint16_t vgaCreateChar(const char c, const vga_color_attributes_t color)
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
static void vgaWriteCharAt(const unsigned x, const unsigned y, const uint16_t c)
{
    if (gVideoMemPtr == nullptr) return;
    if (x >= VGA_WIDTH || y >= VGA_HEIGHT) return;
    gVideoMemPtr[y * VGA_WIDTH + x] = c;
}

/**
 * Enables the hardware text-mode cursor by setting its starting and ending scanline positions.
 *
 * @param cursorStart The starting scanline of the cursor.
 * @param cursorEnd The ending scanline of the cursor.
 */
static void vgaEnableCursor(const uint8_t cursorStart, const uint8_t cursorEnd)
{
    funos::io::outb(0x3D4, 0x0A);
    funos::io::outb(0x3D5, (funos::io::inb(0x3D5) & 0xC0) | cursorStart);

    funos::io::outb(0x3D4, 0x0B);
    funos::io::outb(0x3D5, (funos::io::inb(0x3D5) & 0xE0) | cursorEnd);
}

/**
 * Updates the cursor position on the VGA text-mode display.
 */
static void vgaUpdateCursorPosition()
{
    const uint16_t pos = (gCursorRowPos * VGA_WIDTH) + gCursorColumnPos;

    if (pos >= VGA_WIDTH * VGA_HEIGHT) return;

    funos::io::outb(0x3D4, 0x0F);
    funos::io::outb(0x3D5, pos & 0xFF);
    funos::io::outb(0x3D4, 0x0E);
    funos::io::outb(0x3D5, (pos >> 8) & 0xFF);
}

/**
 * Clears a single line on the display by writing blank characters with the
 * default background and foreground colors.
 *
 * @param y The y-coordinate of the line to be cleared.
 * The value must be within the vertical bounds of the display.
 */
static void displayClearLine(const unsigned y)
{
    const uint16_t black = vgaCreateChar(0x0, DEFAULT_TEXT_COLORS);
    for (int x = 0; x < VGA_WIDTH; x++) vgaWriteCharAt(x, y, black);
}

/**
 * Scrolls the contents of the display up by one line.
 *
 * Copies all lines of the video memory buffer up one position and erases the
 * bottom-most line by filling it with blank spaces.
 * This effectively shifts all displayed content up one row, making space for
 * new content at the bottom.
 */
static void displayScrollUp()
{
    const uint16_t* src = nullptr;
    uint16_t* dest = nullptr;

    for (int y = 0; y < VGA_HEIGHT - 1; y++)
    {
        src = &gVideoMemPtr[(y + 1) * VGA_WIDTH];
        dest = &gVideoMemPtr[y * VGA_WIDTH];

        for (int x = 0; x < VGA_WIDTH; x++) dest[x] = src[x];
    }
    displayClearLine(VGA_HEIGHT - 1);
}

/**
 * Handles line wrapping for the cursor in a VGA text mode display.
 *
 * Resets the cursor's column position to 0 when it reaches the end of a line.
 * If the cursor's row position is at the last row of the screen, the display is scrolled up.
 * Otherwise, the cursor moves to the next row.
 * The new line where the cursor moves is cleared to prepare for further text.
 */
static void cursorHandleLineWrap()
{
    gCursorColumnPos = 0;

    if (gCursorRowPos >= VGA_HEIGHT - 1) displayScrollUp();
    else gCursorRowPos++;

    displayClearLine(gCursorRowPos);
}

/**
 * Writes a single character with specified color attributes to the display.
 * Handles line wrapping and newline characters appropriately.
 *
 * @param c The character to be written to the display.
 * @param color The VGA color attributes to apply to the character, including foreground and background colors.
 */
static void displayPutChar(const char c, const vga_color_attributes_t color)
{
    const uint16_t charCode = vgaCreateChar(c, color);
    if (c == CHAR_NEWLINE)
    {
        cursorHandleLineWrap();
        return;
    }

    vgaWriteCharAt(gCursorColumnPos, gCursorRowPos, charCode);
    if (++gCursorColumnPos >= VGA_WIDTH) cursorHandleLineWrap();
}

void funos::console::clear()
{
    for (int y = 0; y < VGA_HEIGHT; y++) displayClearLine(y);
}

int funos::console::setCursorPosition(const unsigned x, const unsigned y)
{
    if (x >= VGA_WIDTH || y >= VGA_HEIGHT) return -1;

    gCursorColumnPos = x;
    gCursorRowPos = y;
    vgaUpdateCursorPosition();
    return 0;
}

void funos::console::initialize()
{
    gVideoMemPtr = reinterpret_cast<uint16_t*>(VGA_MEMORY_ADDRESS);
    gCursorRowPos = 0;
    gCursorColumnPos = 0;
    clear();
    vgaEnableCursor(14, 15);
    vgaUpdateCursorPosition();
}

void funos::console::writeString(const char* str)
{
    const size_t len = stringLength(str);
    for (size_t i = 0; i < len; i++) displayPutChar(str[i], DEFAULT_TEXT_COLORS);
    vgaUpdateCursorPosition();
}

void funos::console::writeUint(const unsigned value)
{
    char buf[20];
    int pos = 0;
    unsigned vCopy = value;

    if (value == 0)
    {
        displayPutChar('0', DEFAULT_TEXT_COLORS);
        vgaUpdateCursorPosition();
        return;
    }

    // Build the number backwards.
    while (vCopy > 0)
    {
        buf[pos++] = static_cast<char>('0' + (vCopy % 10));
        vCopy /= 10;
    }

    // Display the numbers in the correct order
    while (pos > 0) displayPutChar(buf[--pos], DEFAULT_TEXT_COLORS);
    vgaUpdateCursorPosition();
}

void funos::console::writeInt(const int value)
{
    if (value & INT_SIGN_BIT_MASK)
    {
        displayPutChar('-', DEFAULT_TEXT_COLORS);
        writeUint((~static_cast<unsigned>(value)) + 1);
        return;
    }

    writeUint(value);
}
