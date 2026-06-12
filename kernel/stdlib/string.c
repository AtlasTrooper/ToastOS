#include "string.h"

size_t strlen(const char * str){
  size_t len = 0;
  while(str[len]){
    len+=1;
  }
  return len;

}

int strcmp(const char *a, const char *b) {
    while (*a && *a == *b) { a++; b++; }
    return (unsigned char)*a - (unsigned char)*b;
}

int strncmp(const char *a, const char *b, size_t n) {
    while (n-- && *a && *a == *b) { a++; b++; }
    if (!n) return 0;           // first n chars matched
    return (unsigned char)*a - (unsigned char)*b;
}

void strcpy(char *dst, const char *src) {
    while ((*dst++ = *src++));
}