#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main()
{
  srand(time(NULL));
  char phrases[5][6] = { "hello", "hi", "hey", "howdy", "yo" };
  for(int i = 0; i < 10000; i++){
    int num = rand() % 5;
    printf("%s\n", phrases[num]);
  }
  fflush(stdout);
  while(1);
}
