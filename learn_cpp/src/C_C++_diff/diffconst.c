#include <stdio.h>

int main()
{
    const int a = 10;
    int *p = (int *)&a;
    printf("a = %d\n", a);
    *p = 20;
    printf("a = %d\n", a);
    return 0;
}
