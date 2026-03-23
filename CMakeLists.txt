# Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.20)

# Project name
project(FunOs LANGUAGES C CXX ASM)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Set the cross-compiler toolchain variables
set(TARGET i686-elf)
set(CROSS_COMPILER_PATH "$ENV{HOME}/opt/cross") # Adjust the path as needed
set(CMAKE_C_COMPILER "${CROSS_COMPILER_PATH}/bin/${TARGET}-gcc")
set(CMAKE_CXX_COMPILER "${CROSS_COMPILER_PATH}/bin/${TARGET}-g++")
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
        -fno-builtin
        -Werror
        -Wno-unused-label
        -Wno-cpp
        -Wno-unused-parameter
        -nostdlib
        -nostartfiles
        -nodefaultlibs
        -Wall
        -O0
)

# Add custom build flags for CPP compiler only
set(KERNEL_CXX_FLAGS
        ${KERNEL_C_FLAGS}
        -fno-exceptions
        -fno-unwind-tables
        -fno-rtti
        -fno-use-cxa-atexit
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
        # Add additional assembly files here
        # ${CMAKE_SOURCE_DIR}/src/x86/another_file.asm
)
# Define include directories
set(KERNEL_INCLUDE_DIRS
        ${CMAKE_SOURCE_DIR}/src/x86
        ${CMAKE_SOURCE_DIR}/src/x86/terminal/
        ${CMAKE_SOURCE_DIR}/src/x86/io/
        ${CMAKE_SOURCE_DIR}/src/x86/idt
        ${CMAKE_SOURCE_DIR}/src/x86/memory
        ${CMAKE_SOURCE_DIR}/src/x86/heap/
        ${CMAKE_SOURCE_DIR}/src/x86/disk/
        # Add additional include directories here
)
# Define kernel C source files as a list
set(KERNEL_C_SRC
        # Add additional C files here
        # ${CMAKE_SOURCE_DIR}/src/x86/another_file.c
)
# Define kernel CPP source files as a list
set(KERNEL_CPP_SRC
        ${CMAKE_SOURCE_DIR}/src/x86/kernel.cpp
        ${CMAKE_SOURCE_DIR}/src/x86/terminal/print.cpp
        ${CMAKE_SOURCE_DIR}/src/x86/io/io.cpp
        ${CMAKE_SOURCE_DIR}/src/x86/idt/idt.cpp
        ${CMAKE_SOURCE_DIR}/src/x86/memory/memory.cpp
        ${CMAKE_SOURCE_DIR}/src/x86/memory/paging.cpp
        ${CMAKE_SOURCE_DIR}/src/x86/memory/kpaging.cpp
        ${CMAKE_SOURCE_DIR}/src/x86/heap/heap.cpp
        ${CMAKE_SOURCE_DIR}/src/x86/heap/kheap.cpp
        ${CMAKE_SOURCE_DIR}/src/x86/disk/disk.cpp
        # Add additional C files here
        # ${CMAKE_SOURCE_DIR}/src/x86/another_file.cpp
)
set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/src/linker.ld)
set(LINKER_SCRIPT_DEBUG ${CMAKE_SOURCE_DIR}/src/linker-debug.ld)

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
            COMMAND ${CMAKE_C_COMPILER} ${KERNEL_C_FLAGS} ${INCLUDE_FLAGS} -std=gnu${CMAKE_C_STANDARD} -c ${C_SRC} -o ${C_OBJ}
            DEPENDS ${C_SRC}
            COMMENT "Building C object: ${C_NAME}"
    )
    list(APPEND KERNEL_C_OBJ_FILES ${C_OBJ})
endforeach ()

# Generate CPP object files
set(KERNEL_CPP_OBJ_FILES "")
foreach (CPP_SRC ${KERNEL_CPP_SRC})
    get_filename_component(CPP_NAME ${CPP_SRC} NAME_WE)
    set(CPP_OBJ "${BUILD_DIR}/${CPP_NAME}.cpp.o")
    add_custom_command(
            OUTPUT ${CPP_OBJ}
            COMMAND ${CMAKE_CXX_COMPILER} ${KERNEL_CXX_FLAGS} ${INCLUDE_FLAGS} -std=gnu++${CMAKE_CXX_STANDARD} -c ${CPP_SRC} -o ${CPP_OBJ}
            DEPENDS ${CPP_SRC}
            COMMENT "Building CPP object: ${CPP_NAME}"
    )
    list(APPEND KERNEL_CPP_OBJ_FILES ${CPP_OBJ})
endforeach ()

# Link kernel objects
add_custom_command(
        OUTPUT ${BUILD_DIR}/kernelfull.o
        COMMAND ${CMAKE_LINKER} -relocatable ${KERNEL_ASM_OBJ_FILES} ${KERNEL_C_OBJ_FILES} ${KERNEL_CPP_OBJ_FILES} -o ${BUILD_DIR}/kernelfull.o
        DEPENDS ${KERNEL_ASM_OBJ_FILES} ${KERNEL_C_OBJ_FILES} ${KERNEL_CPP_OBJ_FILES}
        COMMENT "Linking kernel objects"
)

# Create GDB debug kernel binary
add_custom_command(
        OUTPUT ${BIN_DIR}/kernel-debug.elf
        COMMAND ${CMAKE_C_COMPILER} ${KERNEL_C_FLAGS} -T ${LINKER_SCRIPT_DEBUG} -o ${BIN_DIR}/kernel-debug.elf ${BUILD_DIR}/kernelfull.o
        DEPENDS ${BUILD_DIR}/kernelfull.o ${LINKER_SCRIPT_DEBUG}
        COMMENT "Building debug ELF kernel"
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
        COMMAND dd if=/dev/zero bs=512 count=192 >> ${BIN_DIR}/os.bin
        DEPENDS ${BIN_DIR}/boot.bin ${BIN_DIR}/kernel.bin
        COMMENT "Creating OS image (os.bin)"
)

# Main target
add_custom_target(build_x86
        DEPENDS ${BIN_DIR}/os.bin ${BIN_DIR}/kernel-debug.elf
)

# Clean target
add_custom_target(cleanup
        COMMAND rm -rf ${BIN_DIR}/*
        COMMAND rm -rf ${BUILD_DIR}/*
        COMMENT "Cleaning build files"
)

# Create a dummy target for CLion to recognize source files
add_executable(kernel_ide ${KERNEL_C_SRC} ${KERNEL_CPP_SRC} ${KERNEL_ASM_SRC})
target_compile_options(kernel_ide PRIVATE
        $<$<COMPILE_LANGUAGE:C>:${KERNEL_C_FLAGS} -std=gnu${CMAKE_C_STANDARD}>
        $<$<COMPILE_LANGUAGE:CXX>:${KERNEL_CXX_FLAGS} -std=gnu++${CMAKE_CXX_STANDARD}>
)
target_include_directories(kernel_ide PRIVATE ${KERNEL_INCLUDE_DIRS})
target_compile_definitions(kernel_ide PRIVATE
        __FREESTANDING__
        _FREESTANDING
)
set_target_properties(kernel_ide PROPERTIES EXCLUDE_FROM_ALL TRUE EXCLUDE_FROM_DEFAULT_BUILD TRUE)