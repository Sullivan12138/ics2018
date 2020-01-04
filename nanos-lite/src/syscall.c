#include "common.h"
#include "syscall.h"
#include "fs.h"
#include <am.h>
#include <stdlib.h>
extern char end;
uintptr_t sys_yield() {
  _yield();
  return 0;
}

void sys_exit(int code) {
  _halt(code);
}


size_t sys_write(int fd, void *buf, size_t count) {
  if(fd == 1 || fd == 2) {
    char *str = buf;
    int i = 0;
    for(;i< count; i++) _putc(*(str+i));
  }
  return count;
}
uintptr_t sys_brk(void* address) {
  end = *((char*)address);
  return 0;
}
_Context* do_syscall(_Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;
  switch (a[0]) {
    case SYS_yield: c->GPRx = sys_yield(); break;
    case SYS_exit: sys_exit(a[1]); break;
    case SYS_write: c->GPRx = sys_write(a[1], (void *)a[2], a[3]); break;
    case SYS_brk: c->GPRx = sys_brk(a[1]);
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
  return NULL;
}

