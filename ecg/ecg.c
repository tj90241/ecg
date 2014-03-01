//
// ecg.c: Embedded code generator.
//
// ECG: Emulator Code Generator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "ecg/ecg.h"

#ifdef _POSIX_SOURCE
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#ifdef _MSC_VER
HANDLE ecg_hheap = NULL;
#endif

#ifdef _POSIX_SOURCE
long ecg_page_size = 0;
int ecg_zero_page = 0;
#endif


// Uninitializes any globally-acquired data.
int ecg_global_cleanup(void) {
#ifdef _MSC_VER
  if (HeapDestroy(ecg_hheap) == 0)
    return -1;

  ecg_hheap = NULL;
#endif

#ifdef _POSIX_SOURCE
  if (close(ecg_zero_page) != 0) {
    dperror("close");
    return -1;
  }

  ecg_zero_page = 0;
#endif

  return 0;
}

// Allocates and initializes non-const structures.
int ecg_global_init(void) {
#ifdef _MSC_VER
  if ((ecg_hheap = HeapCreate(HEAP_CREATE_ENABLE_EXECUTE, 0, 0)) == NULL)
    return -1;
#endif

#ifdef _POSIX_SOURCE
  ecg_page_size = sysconf(_SC_PAGESIZE);

  if ((ecg_zero_page = open("/dev/zero", O_RDWR)) == -1) {
    dperror("open");
    return -1;
  }
#endif

  return 0;
}

