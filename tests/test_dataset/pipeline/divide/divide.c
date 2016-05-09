#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main( int argc, char *argv[] )
{
    int dividend, divisor;
    int dividend_u, divisor_u;
    int hi, counter, quotient, remainder;
    unsigned int lo;
    
    dividend = atoi( argv[1] );
    divisor = atoi( argv[2] );
    
    if( divisor == 0 ) {
        fprintf( stderr, "try to divide by 0\n" );
        exit( -1 );
    }
    
    dividend_u = abs( dividend );
    divisor_u = abs( divisor );
    
    hi = 0;
    lo = dividend_u;
    counter = 32;
    
    hi = ( ( lo & 0x80000000 ) >> 31 );
    lo <<= 1;
    
    while( counter > 0 ) {
        hi -= divisor_u;
        if( hi < 0 ) {
            hi += divisor_u;
            hi <<= 1;
            hi |= ( ( lo & 0x80000000 ) >> 31 );
            lo <<= 1;
        }
        else {
            hi <<= 1;
            hi |= ( ( lo & 0x80000000 ) >> 31 );
            lo <<= 1;
            lo |= 1;
        }
        counter--;
    }
    
    hi >>= 1;
    
    remainder = hi;
    quotient = lo;
    if( dividend < 0 ) {
        remainder *= -1;
    }
    if( ( dividend & 0x80000000 ) != ( divisor & 0x80000000 ) ) {
        quotient *= -1;
    }
    
    fprintf( stdout, "0x%08x = 0x%08x * 0x%08x + 0x%08x\n", 
                    dividend, quotient, divisor, remainder );

    fprintf( stdout, "%d = %d * %d + %d\n", 
                    dividend, quotient, divisor, remainder );
    
    while( !getchar() );
    return 0;
}
