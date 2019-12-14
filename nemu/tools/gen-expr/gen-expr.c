#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
uint32_t choose(uint32_t n) {
  srand(time(NULL));
  return rand()%n;
}
static char buf[65536];
void gen_rand_op() {
  switch(choose(4)) {
    case 0: {
      strcat(buf, "+");
    };
    case 1: {
      strcat(buf, "-");
    }
    case 2: strcat(buf, "*");
    case 3: strcat(buf, "/");
  }
}  
static inline void gen_rand_expr() {
  switch(choose(3)) {
    case 0: {
      char str[32];
      uint32_t x;
      srand(time(NULL));
      x = rand();
      sprintf(str, "%u", x);
      strcat(buf, str);
      break;
    };
    case 1: {
      strcat(buf, "(");
      gen_rand_expr();
      strcat(buf, ")");
      break;
    };
    default: {
      gen_rand_expr();
      gen_rand_op();
      gen_rand_expr();
    };
  }
}

static char code_buf[65536];
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    gen_rand_expr();

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen(".code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc .code.c -o .expr");
    if (ret != 0) continue;

    fp = popen("./.expr", "r");
    assert(fp != NULL);

    int result;
    fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}
