all:
	nasm -f bin ./src/x86/boot/boot_loader.asm -o ./bin/x86_boot_loader.bin

clean:
	rm ./bin/x86_boot_loader.bin