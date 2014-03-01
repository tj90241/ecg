//
// common.h: Common definitions such.
//
// ECG: Emulator Code Generator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __ecg_common__
#define __ecg_common__

#ifndef __cplusplus
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#else
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#endif

#ifndef NDEBUG
#ifndef __cplusplus
#include <stdio.h>
#else
#include <cstdio>
#endif
#endif

// Define align().
#ifdef _MSC_VER
#define ecg_align(decl, value) __declspec(align(value)) decl

#elif (defined __GNUC__)
#define ecg_align(decl, value) decl __attribute__ ((aligned(value)))

#else
#define ecg_align(decl, value) decl value
#endif

// Define barrier().
#ifdef _MSC_VER
#define inline __inline
#include <windows.h>
#define barrier() _ReadWriteBarrier();

#elif (defined __GNUC__)
#define barrier() __asm__ __volatile__("" ::: "memory");

#else
#warn "barrier() not defined."
#define barrier()
#endif

// Define dperror().
#if (defined(_POSIX_SOURCE) && !defined(NDEBUG))
#define dperror(arg) (perror(arg))
#else
#define dperror(arg)
#endif

struct ecg_ctx {
  uint8_t *buffer;
  uint8_t *end;
  size_t size;

  uint32_t used_regs;
};

#endif

