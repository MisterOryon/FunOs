//
// Created by oryon on 4/26/25.
//

#ifndef IO_H
#define IO_H

/**
 * @brief Reads a byte from the specified I/O port.
 * @param port The I/O port address from which the byte is to be read.
 * @return The byte read from the specified I/O port.
 */
unsigned char io_inb(unsigned short port);

/**
 * @brief Reads a word from the specified I/O port.
 * @param port The I/O port address from which the word is to be read.
 * @return The word (16 bits) read from the specified I/O port.
 */
unsigned short io_inw(unsigned short port);

/**
 * @brief Writes a byte to the specified I/O port.
 * @param port The I/O port address to which the byte is to be written.
 * @param data The byte of data to be written to the specified I/O port.
 */
void io_outb(unsigned short port, unsigned char data);

/**
 * @brief Writes a word (2 bytes) to the specified I/O port.
 * @param port The I/O port address to which the word is to be written.
 * @param data The 16-bit data word to be written to the specified I/O port.
 */
void io_outw(unsigned short port, unsigned short data);

#endif //IO_H
