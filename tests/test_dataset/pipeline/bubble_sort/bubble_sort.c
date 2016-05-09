#include <stdio.h>
#include <stdlib.h>

#define ARRAY_NUM 5

int main( int argc, char *argv[] )
{
	int array[ARRAY_NUM] = { 1, 4, 3, 5, 2 };
	int i, j, k, swap;
/*
	fprintf( stdout, "before sorting: " );
	for( i = 0; i < ARRAY_NUM; ++i ) {
		fprintf( stdout, "%d ", array[i] );
	}
	fprintf( stdout, "\n" );
*/
	for( i = ARRAY_NUM - 1; i > 0; --i ) {
//		fprintf( stdout, "path %d\n", ARRAY_NUM - i );
		for( j = 0; j < i; ++j ) {
			if( array[j] > array[j+1] ) {
				swap = array[j];
				array[j] = array[j+1];
				array[j+1] = swap;
			}
/*			fprintf( stdout, "  step %d: ", j+1 );
			for( k = 0; k < ARRAY_NUM; ++k ) {
				fprintf( stdout, "%d ", array[k] );
			}
			fprintf( stdout, "\n" );*/
		} // end for j
	} // end for i
/*
	fprintf( stdout, " after sorting: " );
	for( i = 0; i < ARRAY_NUM; ++i ) {
		fprintf( stdout, "%d ", array[i] );
	}
	fprintf( stdout, "\n" );
*/
	return 0;
}

