#include <dlfcn.h>

#include "nemu.h"
#include "monitor/monitor.h"
#include "diff-test.h"

static void (*ref_difftest_memcpy_from_dut)(paddr_t dest, void *src, size_t n);
static void (*ref_difftest_getregs)(void *c);
static void (*ref_difftest_setregs)(const void *c);
static void (*ref_difftest_exec)(uint64_t n);

static bool is_skip_ref;
static bool is_skip_dut;

static char reg[8][10] = {"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi"};

void difftest_skip_ref() { is_skip_ref = true; }
void difftest_skip_dut() { is_skip_dut = true; }

void init_difftest(char *ref_so_file, long img_size) {
#ifndef DIFF_TEST
  return;
#endif

  assert(ref_so_file != NULL);

  void *handle;
  handle = dlopen(ref_so_file, RTLD_LAZY | RTLD_DEEPBIND);
  assert(handle);

  ref_difftest_memcpy_from_dut = dlsym(handle, "difftest_memcpy_from_dut");
  assert(ref_difftest_memcpy_from_dut);

  ref_difftest_getregs = dlsym(handle, "difftest_getregs");
  assert(ref_difftest_getregs);

  ref_difftest_setregs = dlsym(handle, "difftest_setregs");
  assert(ref_difftest_setregs);

  ref_difftest_exec = dlsym(handle, "difftest_exec");
  assert(ref_difftest_exec);

  void (*ref_difftest_init)(void) = dlsym(handle, "difftest_init");
  assert(ref_difftest_init);

  Log("Differential testing: \33[1;32m%s\33[0m", "ON");
  Log("The result of every instruction will be compared with %s. "
      "This will help you a lot for debugging, but also significantly reduce the performance. "
      "If it is not necessary, you can turn it off in include/common.h.", ref_so_file);

  ref_difftest_init();
  ref_difftest_memcpy_from_dut(ENTRY_START, guest_to_host(ENTRY_START), img_size);
  ref_difftest_setregs(&cpu);
}

void difftest_step(uint32_t eip) {
  CPU_state ref_r;

  if (is_skip_dut) {
    is_skip_dut = false;
    return;
  }

  if (is_skip_ref) {
    // to skip the checking of an instruction, just copy the reg state to reference design
    ref_difftest_setregs(&cpu);
    is_skip_ref = false;
    return;
  }

  ref_difftest_exec(1);
  ref_difftest_getregs(&ref_r);

  // TODO: Check the registers state with the reference design.
  // Set `nemu_state` to `NEMU_ABORT` if they are not the same.
  int i = 0;
  for(i = 0; i < 8; i++) {
    if(cpu.gpr[i]._32 != ref_r.gpr[i]._32) {
      printf("%s is not the same, the right answer is %x while you get %x, right ZF:%d, you get %d\n", reg[i], ref_r.gpr[i]._32, cpu.gpr[i]._32, ref_r.eflags.ZF, cpu.eflags.ZF);
      nemu_state = NEMU_ABORT;
    }
  }
  if(cpu.eip != ref_r.eip) {
    printf("eip is not the same, the right answer is %d while you get %d\n", ref_r.eip, cpu.eip);
    nemu_state = NEMU_ABORT;
  }
  if(cpu.eflags.CF != ref_r.eflags.CF) {
    printf("CF is not the same\n, the right answer is %d while you get %d\n", ref_r.eflags.CF, cpu.eflags.CF);
    nemu_state = NEMU_ABORT;
  }
  if(cpu.eflags.ZF != ref_r.eflags.ZF) {
    printf("ZF is not the same\n, the right answer is %d while you get %d\n", ref_r.eflags.ZF, cpu.eflags.ZF);
    nemu_state = NEMU_ABORT;
  }
  if(cpu.eflags.SF != ref_r.eflags.SF) {
    printf("SF is not the same\n, the right answer is %d while you get %d\n", ref_r.eflags.SF, cpu.eflags.SF);
    nemu_state = NEMU_ABORT;
  }
  if(cpu.eflags.OF != ref_r.eflags.OF) {
    printf("OF is not the same\n, the right answer is %d while you get %d\n", ref_r.eflags.OF, cpu.eflags.OF);
    nemu_state = NEMU_ABORT;
  }
  
}
