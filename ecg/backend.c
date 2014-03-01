//
// backend.c: Common generator/backend code.
//
// ECG: Emulator Code Generator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "ecg/ecg.h"
#include "ecg/backend.h"

#ifdef _POSIX_SOURCE
#include <sys/mman.h>
#endif

// Allocates an executable buffer on the heap.
size_t ecg_alloc_buf(uint8_t **buffer, size_t size) {
#ifdef _MSC_VER
  *buffer = (uint8_t*) HeapAlloc(ecg_hheap, 0, size);
#endif

#ifdef _POSIX_SOURCE
  int prot = PROT_READ | PROT_WRITE | PROT_EXEC;
  int flags = MAP_SHARED;

  // Allocate full pages only since we're going
  // to allocate pages at page boundaries anyways.
  size = (size + ecg_page_size) & ~(ecg_page_size - 1);
  *buffer = (uint8_t*) mmap(NULL, size, prot,
    flags, ecg_zero_page, 0);

  if (*buffer == MAP_FAILED) {
    dperror("mmap");
    return 0;
  }
#endif

  return size;
}

// Frees memory allocated with ecg_alloc_buf
int ecg_free_buf(uint8_t *buffer, size_t size) {
#ifdef _MSC_VER
  if (HeapFree(ecg_hheap, 0, buffer) == 0)
    return -1;
#endif

#ifdef _POSIX_SOURCE
  if (munmap(buffer, size) != 0) {
    dperror("munmap");
    return -1;
  }
#endif

  return 0;
}

