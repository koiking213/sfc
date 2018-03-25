#include <stdio.h>

void _print(int count, void **elements)
{
  // assume all elements are integer
  for (int i=0; i<count; i++) {
    printf("%d ", *(int *)(elements[i]));
  }
  puts("");
}
