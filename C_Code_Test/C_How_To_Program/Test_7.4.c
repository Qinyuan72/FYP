#include <stdio.h>
int main(){
    int a;
    int *aPtr;

    a = 7;
    aPtr = &a;

    printf( "The address of a in %p\n"
            "The value of *aPtr is %p\n\n", &a, &aPtr);

    printf( "The address of a in %d\n"
            "The value of *aPtr is %d\n\n", &a, &aPtr);

    printf( "Proving that * and & are complemt"
            "\n*aPtr = %p\n*aPtr = %p", &*aPtr, *&aPtr);
}
