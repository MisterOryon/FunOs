FILES = ./build/kernel.asm.o

all: ./bin/boot.bin ./bin/kernel.bin
	rm -rf bin/os.bin
	dd if=./bin/boot.bin >> ./bin/os.bin
	dd if=./bin/kernel.bin >> ./bin/os.bin
	dd if=/dev/zero bs=512 count=100 >> ./bin/os.bin

./bin/kernel.bin: $(FILES)
	i686-elf-ld -g -relocatable $(FILES) -o ./build/kernelfull.o
	i686-elf-gcc -T ./src/linker.ld -o ./bin/kernel.bin -ffreestanding -Oo -nostdlib ./build/kernelfull.o

./bin/boot.bin: ./src/x86/boot/boot_loader.asm
	nasm -f bin ./src/x86/boot/boot_loader.asm -o ./bin/boot.bin

./build/kernel.asm.o: ./src/x86/kernel.asm
	nasm -f elf -g ./src/x86/kernel.asm -o ./build/kernel.asm.o

clean:
	rm -rf ./bin/boot.bin
	rm -rf ./bin/kernel.bin
	rm -rf ${FILES}
	rm -rf ./build/kernelfull.o
