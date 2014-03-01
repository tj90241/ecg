//
// tests/dummy.c: Demonstrates building and calling a ECG context. 
//
// ECG: Emulator Code Generator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "ecg/ecg.h"

int main() {
  struct ecg_ctx ctx;
  uint8_t *buffer;
  size_t size;

  ecg_global_init();

  // We'll manually allocate memory for this.
  size = ecg_alloc_buf(&buffer, 1);
  ecg_ctx_init(&ctx, buffer, size);

  ecg_gen_ret(&ctx);
  ecg_exec_fast(&ctx, NULL);

  ecg_free_buf(buffer, size);
  ecg_global_cleanup();
  return 0;
}

