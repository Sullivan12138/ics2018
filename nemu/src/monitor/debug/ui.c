#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);
CPU_state cpu;
uint8_t pmem[];
/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);
static int cmd_si(char *args) {
  int n = 1;
  if (args != NULL) { 
    if (sscanf(args, "%d", &n) == EOF) {
      printf("Please enter a number as argument.\n");
      return 0;
    }
  }
  if (n <= 0) {
    printf("Please enter a number > 0.\n");
    return 0;
  }
  cpu_exec(n);
  return 0;
}
static int cmd_info(char *args){
  if (strcmp(args, "r") == 0) {
    printf("eax: %#010x\necx: %#010x\nedx: %#010x\nebx: %#010x\nesp: %#010x\nebp: %#010x\nesi: %#010x\nedi: %#010x\n", cpu.eax, cpu.ecx, cpu.edx, cpu.ebx, cpu.esp, cpu.ebp, cpu.esi, cpu.edi);
  }
  else if (strcmp(args, "w") == 0) {
  }
  else printf("Please choose r or w to be argument.\n");
  return 0;
}
static int cmd_p(){}
static int cmd_x(char *args){
  int n = 0;
  char *number, *expr;
  number = strtok(args, " ");
  if (sscanf(number, "%d", &n) != EOF) {
    if (n <= 0) {
      printf("Please enter a positive number N.\n");
      return 0;
    }
  }
  else {
    printf("Please enter a number as the first argument.\n");
    return 0;
  }
  expr = strtok(NULL, " ");
  int addr = 0;
  if (sscanf(expr, "%x", &addr) != EOF) {
    if (n < 0) {
      printf("Please enter an expr that has a value >= 0.\n");
      return 0;
    }
  }
  else {
    printf("Please enter an expr in 0x format.\n");
    return 0;
  }
  for (; n >=0; n--) {
    //printf("%#x%#x%#x%#x", pmem[addr], pmem[addr+1], pmem[addr+2], pmem[addr+3]);
    printf("%c%c%c%c", pmem[addr], pmem[addr+1], pmem[addr+2], pmem[addr+3]);

    addr += 4;
  }
  printf("\n");
  return 0;
}
static int cmd_w(){}
static int cmd_d(){}

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Usage: si [N]. Let the procedure execute N instructions step by step", cmd_si},
  { "info", "Usage: info SUBCMD. Print information. For example, when the SUMCMD is r it means print register state, when the SUBCMD is w it means print watchpoint information.", cmd_info},
  { "p", "Usage: p EXPR. Calculate the value of EXPR", cmd_p},
  { "x", "Usage: x N EXPR. Calculate the value of EXPR, then use it as the start memory address, print N four-bytes in 0x format", cmd_x},
  { "w", "Usage: w EXPR. When EXPR's value changes, pause the program.", cmd_w},
  { "d", "Usage: d N. Delete the watchpoint of sequence number N", cmd_d},
  /* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
