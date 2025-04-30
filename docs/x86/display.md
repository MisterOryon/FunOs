# Display

In FunOs, you have access to three functions for printing data to the screen `console_write_<string, uint, int>`.
To use them import `#include <terminal/print.h>` in your code.

You can also clear the terminal screen with `display_clear`.

If needed, you can update the current cursor position with `display_set_cursor_position`.
After calling this function, all writing to the console starts at the specified position.

Note that in some cases if a bug occurs in the internal of terminal functions with position, FunOs will
quietly discard the character output or cursor position update.
This is to ensure that it doesn't write anything to out-of-range video memory space.