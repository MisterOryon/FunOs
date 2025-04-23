FILES = ./build/kernel.asm.o ./build/kernel.o ./build/config.o ./build/idt/idt.asm.o ./build/idt/idt.o ./build/memory/memory.o
INCLUDES = -I./src/x86
FLAGS = -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc
KERNEL_BIN = ./bin/os.bin

all: ./bin/boot.bin ./bin/kernel.bin
	rm -rf $(KERNEL_BIN)
	dd if=./bin/boot.bin >> $(KERNEL_BIN)
	dd if=./bin/kernel.bin >> $(KERNEL_BIN)
	dd if=/dev/zero bs=512 count=100 >> $(KERNEL_BIN)

./bin/kernel.bin: $(FILES)
	i686-elf-ld -g -relocatable $(FILES) -o ./build/kernelfull.o
	i686-elf-gcc $(FLAGS) -T ./src/linker.ld -o ./bin/kernel.bin -ffreestanding -Oo -nostdlib ./build/kernelfull.o

./bin/boot.bin: ./src/x86/boot/boot_loader.asm
	nasm -f bin ./src/x86/boot/boot_loader.asm -o ./bin/boot.bin

./build/kernel.asm.o: ./src/x86/kernel.asm
	nasm -f elf -g ./src/x86/kernel.asm -o ./build/kernel.asm.o

./build/kernel.o: ./src/x86/kernel.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) -std=gnu99 -c ./src/x86/kernel.c -o ./build/kernel.o

./build/config.o: ./src/x86/config.c
	i686-elf-gcc $(INCLUDES) -I./src/config $(FLAGS) -std=gnu99 -c ./src/x86/config.c -o ./build/config.o

./build/idt/idt.asm.o: ./src/x86/idt/idt.asm
	nasm -f elf -g ./src/x86/idt/idt.asm -o ./build/idt/idt.asm.o

./build/idt/idt.o: ./src/x86/idt/idt.c
	i686-elf-gcc $(INCLUDES) -I./src/idt $(FLAGS) -std=gnu99 -c ./src/x86/idt/idt.c -o ./build/idt/idt.o

./build/memory/memory.o: ./src/x86/memory/memory.c
	i686-elf-gcc $(INCLUDES) -I./src/memory $(FLAGS) -std=gnu99 -c ./src/x86/memory/memory.c -o ./build/memory/memory.o

run:
	qemu-system-i386 -s -S -hda $(KERNEL_BIN)

clean:
	rm -rf ./bin/boot.bin
	rm -rf ./bin/kernel.bin
	rm -rf $(KERNEL_BIN)
	rm -rf ${FILES}
	rm -rf ./build/kernelfull.o
