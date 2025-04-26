# Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.20)

# Project name
project(FunOs LANGUAGES C ASM)

# Set the cross-compiler toolchain variables
set(TARGET i686-elf)
set(CROSS_COMPILER_PATH "$ENV{HOME}/opt/cross") # Adjust the path as needed
set(CMAKE_C_COMPILER "${CROSS_COMPILER_PATH}/bin/${TARGET}-gcc")
set(CMAKE_LINKER "${CROSS_COMPILER_PATH}/bin/${TARGET}-ld")

# Configure NASM for assembly files
set(CMAKE_ASM_COMPILER nasm)

# Add custom build flags for C compiler only
set(KERNEL_C_FLAGS
        -g
        -ffreestanding
        -falign-jumps
        -falign-functions
        -falign-labels
        -falign-loops
        -fstrength-reduce
        -fomit-frame-pointer
        -finline-functions
        -Wno-unused-function
        -fno-builtin -Werror
        -Wno-unused-label
        -Wno-cpp
        -Wno-unused-parameter
        -nostdlib
        -nostartfiles
        -nodefaultlibs
        -Wall
        -O0
        -Iinc
)

# Output directories
set(BUILD_DIR "${CMAKE_BINARY_DIR}/build")
set(BIN_DIR "${CMAKE_BINARY_DIR}/bin")
file(MAKE_DIRECTORY ${BUILD_DIR} ${BIN_DIR})

# Define the source files
set(BOOTLOADER_SRC ${CMAKE_SOURCE_DIR}/src/x86/boot/boot_loader.asm)
# Define kernel ASM source files as a list
set(KERNEL_ASM_SRC
        ${CMAKE_SOURCE_DIR}/src/x86/kernel.asm
        ${CMAKE_SOURCE_DIR}/src/x86/idt/idt.asm
        ${CMAKE_SOURCE_DIR}/src/x86/io/io.asm
        # Add additional assembly files here
        # ${CMAKE_SOURCE_DIR}/src/x86/another_file.asm
)
# Define include directories
set(KERNEL_INCLUDE_DIRS
        ${CMAKE_SOURCE_DIR}/src/x86
        ${CMAKE_SOURCE_DIR}/src/x86/memory
        ${CMAKE_SOURCE_DIR}/src/x86/idt
        ${CMAKE_SOURCE_DIR}/src/x86/terminal/
        ${CMAKE_SOURCE_DIR}/src/x86/io/
        # Add additional include directories here
)
# Define kernel C source files as a list
set(KERNEL_C_SRC
        ${CMAKE_SOURCE_DIR}/src/x86/kernel.c
        ${CMAKE_SOURCE_DIR}/src/x86/config.c
        ${CMAKE_SOURCE_DIR}/src/x86/memory/memory.c
        ${CMAKE_SOURCE_DIR}/src/x86/idt/idt.c
        ${CMAKE_SOURCE_DIR}/src/x86/terminal/print.c
        ${CMAKE_SOURCE_DIR}/src/x86/io/io.c
        # Add additional C files here
        # ${CMAKE_SOURCE_DIR}/src/x86/another_file.c
)
set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/src/linker.ld)

# Target for bootloader (keep custom command as it requires -f bin format)
add_custom_command(
        OUTPUT ${BIN_DIR}/boot.bin
        COMMAND ${CMAKE_ASM_COMPILER} -f bin ${BOOTLOADER_SRC} -o ${BIN_DIR}/boot.bin
        DEPENDS ${BOOTLOADER_SRC}
        COMMENT "Building bootloader"
)

# Generate assembly object files
set(KERNEL_ASM_OBJ_FILES "")
foreach (ASM_SRC ${KERNEL_ASM_SRC})
    get_filename_component(ASM_NAME ${ASM_SRC} NAME_WE)
    set(ASM_OBJ "${BUILD_DIR}/${ASM_NAME}.asm.o")
    add_custom_command(
            OUTPUT ${ASM_OBJ}
            COMMAND ${CMAKE_ASM_COMPILER} -f elf -g ${ASM_SRC} -o ${ASM_OBJ}
            DEPENDS ${ASM_SRC}
            COMMENT "Building ASM object: ${ASM_NAME}"
    )
    list(APPEND KERNEL_ASM_OBJ_FILES ${ASM_OBJ})
endforeach ()

# Prepare include directory flags
set(INCLUDE_FLAGS "")
foreach (INC_DIR ${KERNEL_INCLUDE_DIRS})
    list(APPEND INCLUDE_FLAGS "-I${INC_DIR}")
endforeach ()

# Generate C object files
set(KERNEL_C_OBJ_FILES "")
foreach (C_SRC ${KERNEL_C_SRC})
    get_filename_component(C_NAME ${C_SRC} NAME_WE)
    set(C_OBJ "${BUILD_DIR}/${C_NAME}.c.o")
    add_custom_command(
            OUTPUT ${C_OBJ}
            COMMAND ${CMAKE_C_COMPILER} ${KERNEL_C_FLAGS} ${INCLUDE_FLAGS} -std=gnu99 -c ${C_SRC} -o ${C_OBJ}
            DEPENDS ${C_SRC}
            COMMENT "Building C object: ${C_NAME}"
    )
    list(APPEND KERNEL_C_OBJ_FILES ${C_OBJ})
endforeach ()

# Link kernel objects
add_custom_command(
        OUTPUT ${BUILD_DIR}/kernelfull.o
        COMMAND ${CMAKE_LINKER} -relocatable ${KERNEL_ASM_OBJ_FILES} ${KERNEL_C_OBJ_FILES} -o ${BUILD_DIR}/kernelfull.o
        DEPENDS ${KERNEL_ASM_OBJ_FILES} ${KERNEL_C_OBJ_FILES}
        COMMENT "Linking kernel objects"
)

# Create final kernel binary
add_custom_command(
        OUTPUT ${BIN_DIR}/kernel.bin
        COMMAND ${CMAKE_C_COMPILER} ${KERNEL_C_FLAGS} -T ${LINKER_SCRIPT} -o ${BIN_DIR}/kernel.bin ${BUILD_DIR}/kernelfull.o
        DEPENDS ${BUILD_DIR}/kernelfull.o ${LINKER_SCRIPT}
        COMMENT "Creating final kernel binary"
)

# OS image target
add_custom_command(
        OUTPUT ${BIN_DIR}/os.bin
        COMMAND rm -f ${BIN_DIR}/os.bin
        COMMAND dd if=${BIN_DIR}/boot.bin >> ${BIN_DIR}/os.bin
        COMMAND dd if=${BIN_DIR}/kernel.bin >> ${BIN_DIR}/os.bin
        COMMAND dd if=/dev/zero bs=512 count=100 >> ${BIN_DIR}/os.bin
        DEPENDS ${BIN_DIR}/boot.bin ${BIN_DIR}/kernel.bin
        COMMENT "Creating OS image (os.bin)"
)

# Main target
add_custom_target(build_x86
        DEPENDS ${BIN_DIR}/os.bin
)

# Clean target
add_custom_target(cleanup
        COMMAND rm -rf ${BIN_DIR}/*
        COMMAND rm -rf ${BUILD_DIR}/*
        COMMENT "Cleaning build files"
)

# Create a dummy target for CLion to recognize source files
add_executable(kernel_ide ${KERNEL_C_SRC} ${KERNEL_ASM_SRC})
target_compile_options(kernel_ide PRIVATE ${KERNEL_C_FLAGS})
target_include_directories(kernel_ide PRIVATE ${KERNEL_INCLUDE_DIRS})
set_target_properties(kernel_ide PROPERTIES EXCLUDE_FROM_ALL TRUE EXCLUDE_FROM_DEFAULT_BUILD TRUE)