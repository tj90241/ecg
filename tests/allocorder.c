//
// tests/allocorder.c: Tests register allocation order.
//
// ECG: Emulator Code Generator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "ecg/ecg.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
  struct ecg_ctx ctx;
  uint8_t *buffer;
  size_t size;

  unsigned *r, *rs;
  unsigned i;

  ecg_global_init();

  // We'll manually allocate memory for this.
  size = ecg_alloc_buf(&buffer, 4096);
  ecg_ctx_init(&ctx, buffer, size);

  r = (unsigned*) malloc(sizeof(r) * num_target_regs);
  rs = (unsigned*) malloc(sizeof(rs) * num_target_regs);

#ifndef NDEBUG
  printf("ecg_alloc_reg():\n");
#endif
  for (i = 0; i < num_target_regs; i++) {
#ifndef NDEBUG
    const char *types[] = {
      "CALLEE SAVE",
      "CALLER SAVE",
      "RETURN VALUE",
    };
#endif
    rs[i] = ecg_alloc_reg(&ctx, r + i);
#ifndef NDEBUG
    printf("%s: %s\n", types[rs[i]], target_reg_names[r[i]]);
#endif
  }

  for (i = 0; i < num_target_regs; i++)
    ecg_free_reg(&ctx, r[i]);

#ifndef NDEBUG
  printf("\necg_alloc_cs_reg():\n");
#endif
  for (i = 0; i < num_caller_save_regs; i++) {
    ecg_alloc_cs_reg(&ctx, r + i);
#ifndef NDEBUG
    printf("%s\n", target_reg_names[r[i]]);
#endif
  }

  for (i = 0; i < num_caller_save_regs; i++)
    ecg_free_reg(&ctx, r[i]);

  free(rs);
  free(r);
  ecg_free_buf(buffer, size);
  ecg_global_cleanup();
  return 0;
}

