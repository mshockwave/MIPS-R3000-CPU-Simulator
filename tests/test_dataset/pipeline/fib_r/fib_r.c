#include <stdio.h>
#include <stdlib.h>

int fibonacciR( int pIndex )
{
    int answer;
    
    if( pIndex <= 1 ) {
        answer = pIndex;
    }    
    else {
        answer = fibonacciR( pIndex - 1 ) + fibonacciR( pIndex - 2 );
    }
    
    return answer;
}

int main( int argc, char *argv[] )
{
    int desired, answer;
    int i;
    
    desired = atoi( argv[1] );

    answer = fibonacciR( desired );
    fprintf( stdout, "Fibonacci[%d] = %d\n", desired, answer );
    
    return 0;
}

