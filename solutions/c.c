#include <stdio.h>

int main(){
  char c;
  while ((c = getchar()) != EOF){
    if (!isspace(c)) putchar(c + 1);
    else putchar(c);
  }
  return 0;
}
