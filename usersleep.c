#include<stdio.h>
#define deepsleep() syscall(323)
main()
{
printf("goodnight , Irene\n");
deepsleep();
printf("oops....woke up! \n");
}
