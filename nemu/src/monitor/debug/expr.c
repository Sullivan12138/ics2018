#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ, TK_NOEQ, NUM, LC, RC, HEX, REG, DEREF, AND, MINUS

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
  {"!=", TK_NOEQ},
  {"-", '-'},           // minus
  {"\\*", '*'},         // multiply
  {"\\/", '/'},         // divide
  {"(0x|0X)([1-9]|[a-f])?([0-9]|[a-f])+", HEX},
  {"[1-9]?[0-9]+", NUM},//number
  {"\\(", LC},          // left closure
  {"\\)", RC},          // right closure
  
  {"\\$(eax|ebx|ecx|edx|esp|ebp|esi|edi|sx|bx|cx|dx|sp|bp|si|di|al|bl|cl|dl|ah|bh|ch|dh|eip)", REG},
  
  
  {"&&", AND}
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
              case NUM: {
                  tokens[nr_token].type = NUM;
                  strcpy(tokens[nr_token].str, value);
                  nr_token++;
                  break;
              };
              case TK_NOTYPE: break;
              case HEX: {
                tokens[nr_token].type = HEX;
                int hexValue;
                sscanf(value, "%x", &hexValue);
                sprintf(tokens[nr_token].str, "%d", hexValue);
                nr_token++;
                break;
              };
              case REG: {
                tokens[nr_token].type = REG;
                int j;
                char regName[10];
                sscanf(value, "$%s", regName);
                if(strcmp(regName, "eip") == 0) sprintf(tokens[nr_token].str, "%d", cpu.eip);
                else {
                  for(j = 0; j < 8; j++) {
                    if(strcmp(regName, regsl[j]) == 0) {
                      sprintf(tokens[nr_token].str, "%d", cpu.gpr[j]._32);
                    }
                    else if(strcmp(regName, regsw[j]) == 0) {
                      sprintf(tokens[nr_token].str, "%d", cpu.gpr[j]._16);
                    }
                    else if(strcmp(regName, regsb[j]) == 0) {
                      sprintf(tokens[nr_token].str, "%d", cpu.gpr[j%4]._8[j/4]);
                    }
                  }
                }
                nr_token++;
                break;
              };
              case '*': {
                if(nr_token == 0 || (tokens[nr_token-1].type != NUM && tokens[nr_token-1].type != RC)) {
                  tokens[nr_token].type = DEREF;
                }
                else tokens[nr_token].type = '*';
                nr_token++;
                break;
              };
              case '-': {
                if(nr_token == 0 || (tokens[nr_token-1].type != NUM && tokens[nr_token-1].type != RC)) {
                  tokens[nr_token].type = MINUS;
                }
                else tokens[nr_token].type = '-';
                nr_token++;
                break;
              };
              default: tokens[nr_token++].type = rules[i].token_type;
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
int checkparentheses(int p, int q) {
  int lcNum, rcNum;
  lcNum = rcNum = 0;
  bool flag = false;
  int i;
  for (i = p; i <= q; i++) {
    if (tokens[i].type == LC) lcNum++;
    else if (tokens[i].type == RC) rcNum++;
    if(rcNum > lcNum) return INT32_MAX;
    if(rcNum == lcNum && i < q) flag = true; 
  }
  if(lcNum != rcNum) return INT32_MAX;
  else if (flag == true) return 0;
  else return 1;
}
int priority(int type) {
  if(type == AND) return 0;
  else if(type == TK_EQ || type == TK_NOEQ) return 1;
  else if (type == '+' || type == '-') return 2;
  else if (type == '*' || type == '/') return 3;
  else if(type == DEREF || type == MINUS) return 4;
  else return 5;
}
int findPrimeOp(int p, int q) {
  int op = p;
  int i;
  bool flag = true;
  for (i = p; i <= q; i++) {
    if (tokens[i].type == LC) flag = false;
    else if(tokens[i].type == RC) flag = true;
    if (flag == false) continue;
    if (tokens[i].type == '+' || tokens[i].type == '-' || tokens[i].type == '*' || tokens[i].type == '/' || tokens[i].type == DEREF
        || tokens[i].type == TK_EQ || tokens[i].type == TK_NOEQ || tokens[i].type == AND) {
      if (priority(tokens[i].type) <= priority(tokens[op].type)) op = i;
    }
  }
  return op;
}
int eval(int p, int q) {
  int flag = 0;
  if (p > q) {
    return INT32_MAX;
  }
  else if (p == q) {
    int value = 0;
    if(sscanf(tokens[p].str, "%d", &value) == EOF) return INT32_MAX;
    return value;
  }
  else {
    flag = checkparentheses(p, q);
    if (flag == 1) {
      return eval(p + 1, q - 1);
    }
    else if(flag == 0) {
      int op = findPrimeOp(p, q);
      if(tokens[op].type == DEREF || tokens[op].type == MINUS) {
        int res = eval(op+1, q);
        if(res != -1) { 
          if(tokens[op].type == DEREF) return paddr_read(res, 1);
          else return -eval(op+1, q);
        }
        return INT32_MAX;
      }
      int val1 = eval(p, op - 1);
      int val2 = eval(op + 1, q);
      if(val1 == INT32_MAX || val2 == INT32_MAX) return INT32_MAX;
      switch (tokens[op].type) {
        case '+': return val1 + val2;
        case '-': return val1 - val2;
        case '*': return val1 * val2;
        case '/': return val1 / val2;
        case TK_EQ: return val1 == val2;
        case TK_NOEQ: return val1 != val2;
        case AND: return val1 && val2;
        default: {
          printf("The op isn't right.\n");
          return INT32_MAX;
        };
      }
    }
    else {
      printf("The expr's parentheses does not match.\n");
      return INT32_MAX;
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
  int q = nr_token-1;
  unsigned int ret = eval(p, q);
  if(ret == INT32_MAX) {
    *success = false;
    return UINT32_MAX;
  }
  return ret;
}
