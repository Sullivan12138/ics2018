#include "common.h"
#include <amdev.h>

size_t serial_write(const void *buf, size_t offset, size_t len) {
  size_t i;
  for (i = 0; i < len; ++i) {
    _putc(*((char *)buf + i));
  }
  return i;
}

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

size_t events_read(void *buf, size_t offset, size_t len) {
  // int kc = read_key(), l;
  
  // if ((kc & 0xfff) == _KEY_NONE) {
  //   uint32_t ut = uptime();
  //   l = sprintf(buf, "t %d\n", ut);
  // } else {
  //   if (kc & 0x8000) {
  //     l = sprintf(buf, "kd %s\n", keyname[kc & 0xfff]);
  //     if ((kc & 0xfff) == _KEY_F1) {
  //       fg_pcb = pcbs[1];
        
  //     } else if ((kc & 0xfff) == _KEY_F2) {
        
  //       fg_pcb = pcbs[2];
  //     }
  //   } else {
  //     l = sprintf(buf, "ku %s\n", keyname[kc & 0xfff]);
  //   }
  // }
  // return l;
  return 0;
}

static char dispinfo[128] __attribute__((used));

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  strncpy(buf, dispinfo + offset, len);
  return len;
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  int x = (offset / 4) % screen_width();
  int y = (offset / 4) / screen_width();
  draw_sync();
  draw_rect((uint32_t* )buf, x, y, len / 4, 1);
  
  return len;
}

void init_device() {
  Log("Initializing devices...");
  _ioe_init();

  // TODO: print the string to array `dispinfo` with the format
  // described in the Navy-apps convention
  sprintf(dispinfo, "WIDTH:%d\nHEIGHT:%d\n", screen_width(), screen_height());
}
