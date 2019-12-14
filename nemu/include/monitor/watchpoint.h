#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  char buf[65536];
  int value;
  bool isInit;
} WP;
int checkWatchpoint();
#endif
