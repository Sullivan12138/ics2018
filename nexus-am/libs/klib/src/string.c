#include "klib.h"

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t i;
  for(i = 0; s[i] != '\0';) i++;
  return i;
}

char *strcpy(char* dst,const char* src) {
  return strncpy(dst, src, strlen(src));
}

char* strncpy(char* dst, const char* src, size_t n) {
  size_t i;
  for(i = 0; src[i] != '\0' && i < n; i++) dst[i] = src[i]; 
  dst[i] = '\0';
  return dst;
}

char* strcat(char* dst, const char* src) {
  size_t i = strlen(dst);
  size_t j;
  for(j = 0; src[j] != '\0'; j++) {
    dst[i+j] = src[j];
  }
  dst[i+j] = '\0';
  return dst;
}

int strcmp(const char* s1, const char* s2) {
  while(*s1 && *s2) {
    if(*s1 > *s2) return 1;
    else if(*s1 < *s2) return -1;
    s1++;
    s2++;
  }
  if(*s1) return 1;
  else if(*s2) return -1;
  return 0;
}

int strncmp(const char* s1, const char* s2, size_t n) {
  while(*s1 && *s2 && n--) {
    if(*s1 > *s2) return 1;
	else if(*s1 < *s2) return -1;
    s1++;
    s2++;
  }
  if(n != 0) {
	  if(*s1) return 1;
	  else if(*s2) return -1;
  }
  return 0;
}

void* memset(void* v,int c,size_t n) {
  char *head = v;
  while(n--) {
    *head++ = c;
  }
  return v;
}

void* memcpy(void* out, const void* in, size_t n) {
  char *headout = out;
  const char *headin = in;
  while(n--) {
    *headout++ = *headin++;
  }
  return out;
}

int memcmp(const void* s1, const void* s2, size_t n){
  char *head1 = s1;
  char *head2 = s2;
  while(n--) {
    if(*head1 > *head2) return 1;
    else if(*head1 < *head2) return -1;
	head1++;
	head2++;
  }
  return 0;
}

#endif
