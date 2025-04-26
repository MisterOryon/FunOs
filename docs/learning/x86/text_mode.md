# Text Mode

In this mode, the screen displays characters as pixels.  
The screen is mapped to a specific area of memory, often starting at address `0xB8000`.  
Writing to this memory updates the display with the new character.

Each character cell on the screen holds a character and its associated attributes, such as color.  
The width and height of the display are traditionally 80 characters wide and 20 lines tall.

For example:

```c
volatile char* video_memory = (volatile char*) 0xB8000;

video_memory[0] = 'A';       // Write the character 'A' to the first cell
video_memory[1] = 0x07;      // White text on black background
```