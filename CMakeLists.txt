#
# ECG: Emulator Code Generator.
# Copyright (C) 2014, Tyler J. Stachecki.
#
# This file is subject to the terms and conditions defined in
# 'LICENSE', which is part of this source code package.
#

# TODO/FIXME: Configure for multiple architectures.
# Right now, we only support x86_64, so it's okay...

cmake_minimum_required(VERSION 2.6)
project(ecg)

if ("${CMAKE_C_COMPILER_ID}" MATCHES "GNU")
  set(CMAKE_C_FLAGS "-Wall -Wextra -std=c99 -msse2 -D_POSIX_SOURCE")

  set(CMAKE_C_FLAGS_DEBUG "-ggdb3 -g3 -O0")
  set(CMAKE_C_FLAGS_MINSIZEREL "-Os -DNDEBUG")
  set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")
  set(CMAKE_C_FLAGS_RELWITHDEBINFO "-Og")
endif ("${CMAKE_C_COMPILER_ID}" MATCHES "GNU")

include_directories(${PROJECT_SOURCE_DIR})

# Glob all the files together and build a static library.
file(GLOB BACKEND ${PROJECT_SOURCE_DIR}/backends/*.c)
file(GLOB LIB_HEADERS ${PROJECT_SOURCE_DIR}/ecg/*.c)
file(GLOB LIB_SOURCES ${PROJECT_SOURCE_DIR}/ecg/*.c)
add_library(ecg STATIC ${LIB_SOURCES} ${BACKEND})

# Optionally build all of the test programs.
option(BUILD_TESTS "Build test programs?" OFF)

if (BUILD_TESTS)
  file(GLOB TEST_FILES ${PROJECT_SOURCE_DIR}/tests/*.c)

  foreach(SOURCE_FILE ${TEST_FILES})
    get_filename_component(BINARY_NAME ${SOURCE_FILE} NAME_WE)
    add_executable(${BINARY_NAME} ${SOURCE_FILE})
    target_link_libraries(${BINARY_NAME} ecg)
  endforeach(SOURCE_FILE ${TEST_FILES})
endif (BUILD_TESTS)

# Optionally build a shared library.
option (BUILD_SHARED "Build a shared library?" OFF)

if (BUILD_SHARED)
  add_library(ecg1 SHARED ${LIB_SOURCES})
endif (BUILD_SHARED)

# Install all the header files and libraries to their respective locations.
install(FILES ${LIB_HEADERS} DESTINATION include/ecg)
install(TARGETS ecg DESTINATION lib)

if (BUILD_SHARED)
  install(TARGETS ecg1 DESTINATION lib)
endif (BUILD_SHARED)

