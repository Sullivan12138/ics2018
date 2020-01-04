#include "common.h"
#include "syscall.h"
#include <am.h>

_Context* do_syscall(_Context *c) {
  uintptr_t a[4];
  int result = -1;
  a[0] = c->GPR1;

  switch (a[0]) {
    case SYS_yield: _yield(); result = 0; break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
  c->GPRx = result;
  return NULL;
}
