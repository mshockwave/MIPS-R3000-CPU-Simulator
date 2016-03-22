#include <stdio.h>
#include <stdlib.h>

int sumToN( int pN )
{
    if( pN < 2 ) {
        return pN;
    }
    else {
        return pN + sumToN( pN - 1 );
    }
}

int main( int argc, char *argv[] )
{
    int n = 3, ans;
    
    ans = sumToN( n );
    printf( "1 + 2 + ... + n = %d\n", ans );
    
    while( !getchar() );
    return 0;
}
