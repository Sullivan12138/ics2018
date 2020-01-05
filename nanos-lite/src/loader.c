#include "proc.h"
#include "fs.h"
#define DEFAULT_ENTRY 0x4000000
#define MAP_TEST 1
#define MAP_CREATE 2
static uintptr_t loader(PCB *pcb, const char *filename) {
  int fd = fs_open(filename, 0, 0);
  int len = fs_filesz(fd);
  int blen = pcb->as.pgsize;
  uintptr_t s = DEFAULT_ENTRY;
  char buf[blen];
  while(len > 0) {
    void *page_base = new_page(1);
    _map(&pcb->as, (void *)s, page_base, MAP_CREATE);
    fs_read(fd, buf, blen);
    memcpy(page_base, buf, blen);
    s += blen;
    len -= blen;
  }
  pcb->cur_brk = pcb->max_brk = s;
  fs_close(fd);
  return DEFAULT_ENTRY;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  ((void(*)())entry) ();
}

void context_kload(PCB *pcb, void *entry) {
  _Area stack;
  stack.start = pcb->stack;
  stack.end = stack.start + sizeof(pcb->stack);

  pcb->tf = _kcontext(stack, entry, NULL);
}

void context_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);

  _Area stack;
  stack.start = pcb->stack;
  stack.end = stack.start + sizeof(pcb->stack);

  pcb->tf = _ucontext(&pcb->as, stack, stack, (void *)entry, NULL);
}
