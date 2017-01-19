cmake_minimum_required(VERSION 2.8)

# Force external build
if(${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_BINARY_DIR} AND NOT WIN32)
	message(FATAL_ERROR "You can not use CMake to build from the root of it's source tree! Remove the CMakeCache.txt file from this directory, then create a separate directory (either below this directory or elsewhere), and then re-run CMake from there.")
endif(${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_BINARY_DIR} AND NOT WIN32)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

include(profile)
include(toolchain)

project(raptor)

# http://wiki.osdev.org/CMake_Build_System
# This was very helpful in this build system
load_profile("arm" "rpi")
message(STATUS "${CMAKE_C_COMPILER}")

# Get all the files needed
file(GLOB KERN_TTY_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "kern/tty/*.c")
file(GLOB KERN_LIB_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "kern/lib/*.c")
file(GLOB KERN_MM_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "kern/mm/*.c")
file(GLOB KERN_INPUT_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "kern/input/*.c")
file(GLOB KERN_CMOS_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "kern/cmos/*.c")
file(GLOB KERN_DISK_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "kern/disk/*.c")
file(GLOB KERN_TASK_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "kern/task/*.c")
set(KERN_SOURCES ${KERN_TTY_SOURCES} ${KERN_LIB_SOURCES} ${KERN_MM_SOURCES} ${KERN_INPUT_SOURCES} ${KERN_CMOS_SOURCES} ${KERN_DISK_SOURCES} ${KERN_TASK_SOURCES})
# file(GLOB KERN_TTY_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "kern/tty/*.c")
file(GLOB LIBC_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "libc/*.c")

# these are any additional flags which should be added to the base compiles. Things like 32 bit or elf binaries
set(CFLAGS "${ISA_C_FLAGS} -Wall -Wextra -ffreestanding -fno-builtin")
set(CFLAGS "${CFLAGS} -nostdlib -finline-functions -fno-stack-protector -fomit-frame-pointer")
set(CFLAGS "${CFLAGS} -Werror=implicit-function-declaration")
# linker flags and Assembler flags too -Wl,--oformat,binary
set(LINKFLAGS "${ISA_LINKER_FLAGS} ${PLATFORM_LINKER_FLAGS}")
set(ASM_FLAGS "${ISA_ASM_FLAGS}")

include_directories(
	#${CMAKE_SOURCE_DIR}/libc
	${CMAKE_SOURCE_DIR}/kernel
)

set(BASE_CFLAGS "${CFLAGS} -fstrength-reduce -fno-builtin-time -fno-builtin-puts -fno-builtin-printf -fno-builtin-function")
set(LINKFLAGS "${LINKFLAGS}")

set(LINKFLAGS "-T ${PLATFORM_LAYOUT} -N ${LINKFLAGS}")

set(CMAKE_C_FLAGS                "-Wall -g ${BASE_CFLAGS}")
set(CMAKE_C_FLAGS_DEBUG          "-g -D_DEBUG ${BASE_CFLAGS}")
set(CMAKE_C_FLAGS_MINSIZEREL     "-Os -DNDEBUG ${BASE_CFLAGS}")
set(CMAKE_C_FLAGS_RELEASE        "-O3 -DNDEBUG ${BASE_CFLAGS}")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g ${BASE_CFLAGS}")

set(CMAKE_CXX_FLAGS                "-Wall ${BASE_CFLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG          "-g -D_DEBUG ${BASE_CFLAGS}")
set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG ${BASE_CFLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE        "-O3 -DNDEBUG ${BASE_CFLAGS}")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g ${BASE_CFLAGS}")

set(CMAKE_ASM_FLAGS            "-Werror -felf")

# Concatenate and sort
set(SOURCE_FILES ${LIBC_SOURCES} ${KERN_SOURCES} ${ISA_SRCS} ${PLATFORM_SRCS})
list(SORT SOURCE_FILES)

# Assembly compiles
SET(CMAKE_ASM-ATT_COMPILE_OBJECT  "<CMAKE_ASM-ATT_COMPILER> ${ASM_FLAGS} -o <OBJECT> <SOURCE>")

add_executable(${PROJECT_NAME}.bin ${SOURCE_FILES})
set_source_files_properties(${SOURCE_FILES} PROPERTIES LANGUAGE C COMPILE_FLAGS "")
set_target_properties(${PROJECT_NAME}.bin PROPERTIES LINKER_LANGUAGE C PREFIX "" SUFFIX "" LINK_FLAGS "${LINKFLAGS}")
