#include <stdio.h>
#include <stdlib.h>

int add( int a, int b )
{
    return a + b;
}

int main( int argc, char *argv[] )
{
    int sum, a = -1, b = 0x1235;
    
    sum = add( a, b );    
    printf( "sum = 0x%x\n", sum );
    
    return 0;
}
