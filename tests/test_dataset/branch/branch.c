#include <stdio.h>
#include <stdlib.h>

int main( int argc, char *argv[] )
{
    unsigned short x = 0xABCD;
    unsigned char y = 0x80;
    char z = 0x81;
    short k;
    
    if( x == 0 ) {
        if( y <= (unsigned char)0x88 && z >= (char)0x80 ) {
            k = 1;
        }
        else {
            k = -1;
        }
    }
    else {
        if( y > (unsigned char)0x88 || z < (char)0x80 ) {
            k = 2;
        }
        else if( y == (unsigned char)0x88 && z == (char)0x80 ) {
            k = 0;
        }
        else {
            k = -2;
        }
    }
    
    printf( "k = %d\n", k );
    while( !getchar() );
    
    return 0;
}
