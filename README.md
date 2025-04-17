# FunOs

FunOS is a personal project that emerged from a desire to learn, experiment, and create something meaningful: a minimal
operating system developed for both fun and education.

While the ultimate goal is to build a fully operational OS, FunOS is mainly designed as a learning and exploration tool.
I develop it during my free time, and I hope this open-source project can inspire you to dive into kernel
development, whether for learning or for fun.

If you have any questions or would like to discuss anything about FunOS or kernel development, feel free to open a
discussion or issue on GitHub, I'll be more than happy to respond and help!

If you’d like to contribute new ideas or features, you’re welcome to propose them by opening an issue.
While there isn’t a strict guideline for pull requests (PRs) at the moment, here are some recommendations if you’d like
to get involved:

1. **Open an issue before coding**: Share your idea and gather feedback before you begin development.
2. **Document your work**: Remember that this project’s goal is learning, and many contributors (myself included) are
   not experts in kernel development.
   Clear documentation helps us all grow together.
3. **Submit your PR**: Once your code is ready and documented, open a pull request and tag me (@MisterOryon), and I’ll
   do my best to review it and provide assistance.
4. **Celebrate your contribution**: When it’s ready, I’ll merge your code into FunOS :rocket:.
   Congratulations and thank you for contributing :heart:!

Together, we can make FunOS an enjoyable and valuable project for anyone interested in the world of kernel development.

## Requirements

- `nasm`
- `qemu`
- `gdb`
- `make`
- `cross compiler`

### On Arch Linux

Run the following command to install the required tools:
`pacman -Syu nasm qemu-full gdb make base-devel gmp libmpc mpfr`

#### Setting Up the Cross-Compiler

1. **Download the following archives:**
    - `gcc-14.2.0`
    - `binutils-2.44`

2. **Set environment variables for the cross-compiler:**
   ```bash
   export PREFIX="$HOME/opt/cross"
   export TARGET=i686-elf
   export PATH="$PREFIX/bin:$PATH"
   ```

3. **Build and install Binutils:**
   ```bash
   cd $HOME/src
   mkdir build-binutils
   cd build-binutils
   ../binutils-2.44/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
   make
   make install
   ```

   **Check if Binutils is correctly installed:**

    ```bash
    cd $HOME/src
    which -- $TARGET-as || echo "$TARGET-as is not in the PATH"
    ```

4. **Build and install GCC:**

   ```bash
   mkdir build-gcc
   cd build-gcc
   ../gcc-14.2.0/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
   make all-gcc
   make all-target-libgcc
   make install-gcc
   make install-target-libgcc
   ```

5. **Verify the cross-compiler installation:**

   ```bash
   $HOME/opt/cross/bin/$TARGET-gcc --version
   ```

## Build

To build FunOs for x86 run `./build.sh`.

## Run

To run FunOS, use the following command: `qemu-system-x86_64 -hda ./bin/os.bin`

## Run in Debugging Environment

To run FunOS in a debugging environment, use the following command:

```bash
qemu-system-x86_64 -s -S -hda ./bin/os.bin
```

- **`-s`**: Shorthand for `-gdb tcp::1234`, which starts a GDB server listening on TCP port 1234.
- **`-S`**: Makes QEMU stop execution at the very beginning until you tell it to continue using GDB.

This allows you to connect a debugger like GDB to debug FunOS step-by-step.

### GDB

To load debug information, use the following command in GDB:

```bash
add-symbol-file build/kernelfull.o 0x100000
```

To connect to QEMU with GDB, issue the following command inside GDB:

```bash
target remote localhost:1234
```

#### Setting Breakpoints

Place your breakpoint with the command:

```bash
break *<addr>
```

In FunOS, the following address is interesting:

- **0x7c00**: This is the memory address where the bootloader is loaded.
- **_start**: The entry point of `kernel.asm` (main function).

#### Debugging in GDB

1. After placing your breakpoint, use the `continue` command to start execution.
2. Use the `layout asm` command to view the assembly layout.
3. To step through the code instruction by instruction, use the `stepi` command to move to the next instruction.

This approach allows you to debug and understand the step-by-step execution of FunOS.

## Tanks

I would like to thank the great people who contributed to writing documentation and allowing me to learn about kernel
development:

- Thanks to **Daniel McCarthy** for writing the book *"Developing a Multithreaded Kernel from Scratch"*, which I used to
  learn how to build a kernel.
- Thanks to the authors of **osdev.org**, a great resource for learning the specifics of kernel development.
- Thanks to the author of **www.ctyme.com**, a great place to find information about a comprehensive list of available
  BIOS functions.