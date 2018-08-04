#include <stdio.h>

void _write_int(int value)
{
  printf("%d\n", value);
}
void _write_float(float value)
{
  printf("%f\n", value);
}
void _write_logical(int value)
{
  if (value) {
    puts("T");
  } else {
    puts("F");
  }
}
