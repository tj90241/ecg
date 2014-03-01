//
// ecg.h: Embedded code generator.
// Include this in your project to use ecg.
//
// ECG: Emulator Code Generator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __ecg_ecg_h__
#define __ecg_ecg_h__
#include "ecg/common.h"
#include "ecg/backend.h"

#ifdef _MSC_VER
extern HANDLE ecg_hheap;
#endif

#ifdef _POSIX_SOURCE
extern long ecg_page_size;
extern int ecg_zero_page;
#endif

int ecg_global_cleanup(void);
int ecg_global_init(void);

// Executes an ecg block directly; don't bother with prologues, etc.
static inline void *ecg_exec_fast(struct ecg_ctx *ctx, void *ptr) {
  barrier();

  ptr = ((void * (*)(struct ecg_ctx *, void *))ctx->buffer)(ctx, ptr);
  return ptr;
}

#endif

