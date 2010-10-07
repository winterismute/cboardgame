#define NUM_RIGHE 8
#define NUM_COLONNE 8

#define VUOTO 0
#define NERO 1
#define BIANCO 2

#define AI_MAX_DEPTH 7
#define MAX_FIGLI 30

#define MAX_PARTITE_TEST 30

#ifndef max
	#define max( a, b ) ( ((a) > (b)) ? (a) : (b) )
#endif

#ifndef min
	#define min( a, b ) ( ((a) < (b)) ? (a) : (b) )
#endif

//#define crop(x, min, max) ( ((x) <= (min)) ? (min) : ( ( (x) >= (max) ) ? (max) : (x) ) )
