//
// tests/regalloc.c: Demonstrates register allocation and usage.
//
// ECG: Emulator Code Generator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "ecg/ecg.h"
#include <stdio.h>

int main() {
  struct ecg_ctx ctx;
  uint8_t *buffer;
  size_t size;

  unsigned r1, r2;
  int sum;

  ecg_global_init();

  // We'll manually allocate memory for this.
  size = ecg_alloc_buf(&buffer, 4096);
  ecg_ctx_init(&ctx, buffer, size);

  ecg_alloc_reg(&ctx, &r1);
  ecg_alloc_reg(&ctx, &r2);

  ecg_gen_movi32(&ctx, r1, 5);
  ecg_gen_movptr(&ctx, r2, &sum);
  ecg_gen_alu_imm(&ctx, ECG_OPCODE_ADD, r1, -1, ECG_32BIT);
  ecg_gen_st(&ctx, ECG_OPCODE_SW, r1, r2, ECG_32BIT);
  ecg_gen_ret(&ctx);

  ecg_free_reg(&ctx, r1);
  ecg_free_reg(&ctx, r2);

  ecg_exec_fast(&ctx, NULL);
  printf("The sum is: %d\n", sum);

  ecg_free_buf(buffer, size);
  ecg_global_cleanup();
  return 0;
}

