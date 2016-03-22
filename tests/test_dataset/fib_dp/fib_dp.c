#include <stdio.h>
#include <stdlib.h>

int main( int argc, char *argv[] )
{
    int desired, answer, i;
    int p0 = 0, p1 = 1;

    desired = atoi( argv[1] );

    if( desired <= 1 ) {
        answer = desired;
    }
    else {
        for( i = 2; i <= desired; ++i ) {
            answer = p0 + p1;
            p0 = p1;
            p1 = answer;
        }
    }
    
    fprintf( stdout, "Fibonacci[%d] = %d\n", desired, answer );
    
    return 0;
}

