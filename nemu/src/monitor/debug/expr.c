#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ, NUM, LC, RC

  /* TODO: Add more token types */

};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"==", TK_EQ},        // equal
  {"-", '-'},           // minus
  {"\\*", '*'},         // multiply
  {"\\/", '/'},         // divide
  {"[1-9]?[0-9]+", NUM},//number
  {"\\(", LC},          // left closure
  {"\\)", RC}           // right closure
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        int count;
        char value[32];
        for (count = 0; count < substr_len && count < 31; count++) {
          value[count] = *(substr_start + count);
        }
        value[count] = '\0';
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case 0: {
            tokens[nr_token++].type = TK_NOTYPE;
          };
          case 1: {
            tokens[nr_token++].type = '+';
          };
          case 2: {
            tokens[nr_token++].type = TK_EQ;
          };
          case 3: {
            tokens[nr_token++].type = '-';
          };
          case 4: {
            tokens[nr_token++].type = '*';
          };
          case 5: {
            tokens[nr_token++].type = '/';
          };
          case 6: {
            tokens[nr_token].type = NUM;
            strcpy(tokens[nr_token].str, value);
            nr_token++;
          };
          case 7: {
            tokens[nr_token++].type = LC;
          };
          case 8: {
            tokens[nr_token++].type = RC;
          };
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}
bool checkparentheses(int p, int q) {
  if (tokens[p].type != LC || tokens[q].type != RC) return false;
  bool flag = true;
  int i;
  for (i = p; i <= q; i++) {
    if (tokens[i].type == LC) flag = false;
    if (tokens[i].type == RC) {
     if (flag == true)  return false;
     else flag = true;
    }
  }
  if (flag == false) assert(0);
  else return true;
}
int priority(int type) {
  if (type == '+' || type == '-') return 1;
  else if (type == '*' || type == '/') return 2;
  else return 3;
}
int findPrimeOp(int p, int q) {
  int op = p;
  int i;
  bool flag = true;
  for (i = p; i <= q; i++) {
    if (tokens[i].type == LC) flag = false;
    else if(tokens[i].type == RC) flag = true;
    if (flag == false) continue;
    if (tokens[i].type == '+' || tokens[i].type == '-' || tokens[i].type == '*' || tokens[i].type == '/') {
      if (priority(tokens[i].type) <= priority(tokens[op].type)) op = i;
    }
  }
  if (priority(tokens[op].type) == 3) assert(0);
  return op;
}
int eval(int p, int q) {
  if (p > q) {
    assert(0);
  }
  else if (p == q) {
    int value = 0;
    sscanf(tokens[p].str, "%d", &value);
    printf("tokens[%d].str: %s\n", p, tokens[p].str);
    printf("value: %d\n", value);
    return value;
  }
  else if (checkparentheses(p, q) == true) {
    return eval(p + 1, q - 1);
  }
  else {
    int op = findPrimeOp(p, q);
    int val1 = eval(p, op - 1);
    int val2 = eval(op + 1, q);
    switch (tokens[op].type) {
      case '+': return val1 + val2;
      case '-': return val1 - val2;
      case '*': return val1 * val2;
      case '/': return val1 / val2;
      default: assert(0);
    }
  }
}
uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  int p = 0;
  int q = nr_token;
  return eval(p, q);
}
