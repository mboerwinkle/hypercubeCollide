#ifndef GLOBAL_H
#define GLOBAL_H

//dimensions
//#define DIM 3
#ifndef DIM
	#error DIM must be defined
#endif

#if DIM == 2
	//Create diagnostic drawings
	//#define DRAW
#endif



#define RESOLUTION 2 //Minimum magnitude that it will calculate at.
//2^dimensions
#define TWOPOWDIM (1<<DIM)
#if DIM == 2
	#define SQRTDIM 1.414213562373095048801688724209698078569671875376948073176
	#define THREEPOWDIM 9
#elif DIM == 3
	#define SQRTDIM 1.732050807568877293527446341505872366942805253810380628055
	#define THREEPOWDIM 27
#else
	#error "No THREEPOWDIM for unknown DIM"
#endif

//hash table size for the oclass handler
#define OCLASSHASHSIZE 1000
//hash table size for the collision handler
#define COLLISIONHASHSIZE 3000
//hash table size for the list of oinstances
#define OINSTANCEHASHSIZE 1000

typedef struct point{
	int p[DIM];
}point;
#define POINTMAX (2147483647)
#define POINTMIN (-POINTMAX)
typedef struct orientation{
	#if DIM == 2
		double r;//radians
	#elif DIM == 3
		double r[4];//quaternion
	#else
		#error "Orientation not defined for unknown DIM"
	#endif
}orientation;

//geom.c
extern point rotatePoint(point t, orientation r, double mult);
extern double odistance(point a);
extern double distance(point a, point b);
extern double distanceSquared(point a, point b);
extern int factorial[12];
extern int intPow(int b, int x);
extern unsigned long int_sqrt(unsigned long s);
extern int nCr[11][11];
extern void nCrChooseInit();
extern void nCrChoose(int n, int r, int idx, int* w);
extern int hammingWeight(int x);
extern int getDim();
#endif
