/* main.c */

#include <stdio.h>
#include <unistd.h>

int enOceanOpen();
int enOceanClose();

int
main() {
   int ret;

   printf("start enocean test\n");
   ret = enOceanOpen();
   printf("ret = %d\n", ret);

   // keep program running
   while (1) {
       sleep(1);
   }
}
