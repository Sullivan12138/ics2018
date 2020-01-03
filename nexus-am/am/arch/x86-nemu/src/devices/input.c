#include <am.h>
#include <x86.h>
#include <amdev.h>
#define I8042_DATA_PORT 0x60
unsigned long long input;
size_t input_read(uintptr_t reg, void *buf, size_t size) {
  switch (reg) {
    case _DEVREG_INPUT_KBD: {
      _KbdReg *kbd = (_KbdReg *)buf;
      input = inl(I8042_DATA_PORT);
      kbd->keydown = ((input & 0x8000) == 0) ? 0 : 1;
      kbd->keycode = input;
      return sizeof(_KbdReg);
    }
  }
  return 0;
}
