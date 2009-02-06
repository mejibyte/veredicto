#include <stdio.h>
int main(){
  char c;
  while ((c = getchar()) != EOF) putchar(toupper(c));
  return 0;
}
