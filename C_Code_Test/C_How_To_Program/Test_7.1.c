#include <stdio.h>
int main() {
   int y = 5;
   int *yPrt;
   yPrt = &y;
   printf("%d",*yPrt);   
}