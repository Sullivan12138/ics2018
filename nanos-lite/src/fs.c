#include "fs.h"

extern size_t ramdisk_read(void *buf, size_t offset, size_t len);
extern size_t ramdisk_write(const void *buf, size_t offset, size_t len);
extern size_t serial_write(const void *buf, size_t offset, size_t len);

extern size_t events_read(void *buf, size_t offset, size_t len);
extern size_t dispinfo_read(void *buf, size_t offset, size_t len);
extern size_t fb_write(const void *buf, size_t offset, size_t len);

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  size_t open_offset;
  ReadFn read;
  WriteFn write;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR,  FD_FB, FD_EVENTS, FD_DISPINFO, FD_TTY};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  {"stdin", 0, 0, 0, invalid_read, invalid_write},
  {"stdout", 0, 0, 0, invalid_read, serial_write},
  {"stderr", 0, 0, 0, invalid_read, serial_write},
  {"/dev/fb", 0, 0, 0, invalid_read, fb_write},
  {"/dev/events", 0, 0, 0, events_read, invalid_write},
  {"/proc/dispinfo", 128, 0, 0, dispinfo_read, invalid_write},
  {"/dev/tty", 0, 0, 0, invalid_read, serial_write},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

void init_fs() {
  // TODO: initialize the size of /dev/fb
  file_table[FD_FB].size = screen_width() * screen_height() * 4;
  file_table[0].size = file_table[1].size = file_table[2].size= 0x7fffffff;
}

int fs_open(const char *pathname, int flags, int mode) {
  int i = 0;
  for(; i < NR_FILES; i++) {
    if(strcmp(file_table[i].name, pathname) == 0) {
      file_table[i].open_offset = 0;
      return i;
    }
  }
  assert(0);
  return -1;
}

size_t fs_filesz(int fd) {
	return file_table[fd].size;
}

ssize_t fs_read(int fd, void *buf, size_t len) {
  // printf("fs_read:%d\n", fd);
  size_t l = (file_table[fd].open_offset + len) <= fs_filesz(fd) ? len : (fs_filesz(fd) - file_table[fd].open_offset);
  if(file_table[fd].read == NULL) ramdisk_read(buf, file_table[fd].disk_offset + file_table[fd].open_offset, l);
  else l = file_table[fd].read(buf, file_table[fd].disk_offset + file_table[fd].open_offset, l);
  file_table[fd].open_offset += l;
  return l;
}

ssize_t fs_write(int fd, const void *buf, size_t len) {
  size_t l = (file_table[fd].open_offset + len) <= fs_filesz(fd) ? len : (fs_filesz(fd) - file_table[fd].open_offset);
  if(file_table[fd].write == NULL) ramdisk_write(buf, file_table[fd].disk_offset + file_table[fd].open_offset, l);
  else l = file_table[fd].write(buf, file_table[fd].disk_offset + file_table[fd].open_offset, l);
  file_table[fd].open_offset += l;
  return l;
}

off_t fs_lseek(int fd, off_t offset, int whence) {
  switch(whence) {
    case SEEK_SET: assert(offset <= file_table[fd].size); file_table[fd].open_offset = offset; break;
    case SEEK_CUR: assert(file_table[fd].open_offset + offset <= file_table[fd].size); file_table[fd].open_offset += offset; break;
    case SEEK_END: assert(file_table[fd].open_offset <= 0); file_table[fd].open_offset = file_table[fd].size + offset; break;
    default: panic("Unkown whence.\n");
  }
  return file_table[fd].open_offset;
}

int fs_close(int fd) {
  fs_lseek(fd, 0, SEEK_SET);
  return 0;
}