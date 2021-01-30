#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "Global.h"

//#define HAMMING hammingWeight //alternative for non-gcc compiler
#define HAMMING __builtin_popcount

int getDim(){ return DIM; }

#if DIM == 3
orientation quatMult(orientation oa, orientation ob){
	double* a = oa.r;
	double* b = ob.r;
	orientation ret;
	ret.r[0]=(b[0] * a[0] - b[1] * a[1] - b[2] * a[2] - b[3] * a[3]);
	ret.r[1]=(b[0] * a[1] + b[1] * a[0] + b[2] * a[3] - b[3] * a[2]);
	ret.r[2]=(b[0] * a[2] - b[1] * a[3] + b[2] * a[0] + b[3] * a[1]);
	ret.r[3]=(b[0] * a[3] + b[1] * a[2] - b[2] * a[1] + b[3] * a[0]);
	return ret;
}
#endif

point rotatePoint(point t, orientation r, double mult){
#if DIM == 2
	point ret;
	double c = cos(mult*(r.r));
	double s = sin(mult*(r.r));
	ret.p[0]= t.p[0] * c + t.p[1] * s;
	ret.p[1]= -t.p[0] * s + t.p[1] * c;
	return ret;
#elif DIM == 3//FIXME doesn't use mult
	double mag = odistance(t);
	if(mag == 0) return (point) {.p={0, 0, 0}};//To avoid div/0
	orientation pureVec = {.r={0, ((double) t.p[0])/mag, ((double) t.p[1])/mag, ((double) t.p[2])/mag}};
	orientation revRot = {.r={r.r[0], -r.r[1], -r.r[2], -r.r[3]}};
	if(mult == 1){
		pureVec = quatMult(r, pureVec);
		pureVec = quatMult(pureVec, revRot);
	}else if(mult == -1){
		pureVec = quatMult(revRot, pureVec);
		pureVec = quatMult(pureVec, r);
	}else{
		puts("Unsupported multiplier in rotatepoint");
	}
	return (point) {.p={pureVec.r[1]*mag, pureVec.r[2]*mag, pureVec.r[3]*mag}};
#else
	#error "No rotatePoint Function for unknown DIM"
#endif
}

double odistance(point a){
	double ret = 0.0;
	for(int d = 0; d < DIM; d++){
		ret+=a.p[d]*a.p[d];
	}
	return sqrt(ret);
}
double distance(point a, point b){
	double ret = 0.0;
	for(int d = 0; d < DIM; d++){
		ret+=(a.p[d]-b.p[d])*(a.p[d]-b.p[d]);
	}
	return sqrt(ret);
}
double distanceSquared(point a, point b){//make dim-dependent macro. perf shows it might make a difference
	double ret = 0.0;
	for(int d = 0; d < DIM; d++){
		ret+=(a.p[d]-b.p[d])*(a.p[d]-b.p[d]);
	}
	return ret;
}
int factorial[12] = {1, 1, 2, 6, 24, 120, 720, 5040, 40320, 362880, 3628800, 39916800};
/*int factorialData[20] = {1};
int factorialMax = 0;
int factorial(int n){
	if(n > factorialMax){
		for(int idx = factorialMax; idx < n; idx++){
			factorialData[idx+1] = factorialData[idx]*(idx+1);
		}
		factorialMax = n;
	}
	return factorialData[n];
}*/

int nCr[11][11] = {
	{1},												//0
	{1, 1},												//1
	{1, 2, 1},											//2
	{1, 3, 3, 1},										//3
	{1, 4, 6, 4, 1},									//4
	{1, 5, 10, 10, 5, 1},								//5
	{1, 6, 15, 20, 15, 6, 1},							//6
	{1, 7, 21, 35, 35, 21, 7, 1},						//7
	{1, 8, 28, 56, 70, 56, 28, 8, 1},					//8
	{1, 9, 36, 84, 126, 126, 84, 36, 9, 1},				//9
	{1, 10, 45, 120, 210, 252, 210, 120, 45, 10, 1}};	//10

/*int nCr(int n, int r){
	int ret = factorial[n]/(factorial[r]*factorial[n-r]);
	return ret;
}*/

//int nCrChooseInitialized = 0;
#define NCRDATA 6
#define NCRDATA2 10
int nCrData[NCRDATA][NCRDATA2];//This covers all the nCr's we are likely to see. 10 is chosen because it is the max of (5 nCr X).
void nCrChooseInit(){//this creates the ncr choose lookup table. ncrchoose(5, 2, 2) is identical to ncrchoose(3, 2, 2)
	assert(NCRDATA2 == nCr[NCRDATA-1][(NCRDATA-1)/2]); //this is how we calculate the lookup table size.
	assert(NCRDATA > DIM); //If not, then the ncr table isn't big enough to handle this dimension
	for(int r = 0; r < NCRDATA; r++){
		int num = -1;
		for(int idx = 0; idx < nCr[NCRDATA-1][r]; idx++){
			num++;
			while(HAMMING(num) != r){
				num++;
			}
			assert(HAMMING(num) == r);
			assert(r < NCRDATA);
			assert(idx < NCRDATA2); 
			nCrData[r][idx] = num;
		} 
	}
	//nCrChooseInitialized = 1;
}
void nCrChoose(int n, int r, int idx, int* w){//This function returns an int array which is a specific choice (nCr style) (3, 1, 0) is {0, 0, 1}. //FIXME really this should cache results.
	//if(!nCrChooseInitialized) nCrChooseInit();
	assert(nCr[n][r] > idx);
	int num = nCrData[r][idx];
	for(int x = 0; x < n; x++){
		if((num>>x) & 1){
			w[x] = 1;
		}else{
			w[x] = 0;
		}
	}
}
/*void nCrChoose(int n, int r, int idx, int* w){//This function returns an int array which is a specific choice (nCr style) (3, 1, 0) is {0, 0, 1}. //FIXME really this should cache results.
	int num = -1;
	assert(nCr(n, r) > idx);
	for(int x = 0; x <= idx; x++){
		num++;
		while(HAMMING(num) != r){
			num++;
		}
	}
	for(int x = 0; x < n; x++){
		if((num>>x) & 1){
			w[x] = 1;
		}else{
			w[x] = 0;
		}
	}
}*/
int hammingWeight(int x){
	int ret = 0;
	for(int s = 0; s < 32; s++){
		if((x>>s) & 1) ret++;
	}
	assert(__builtin_popcount(x) == ret);
	return ret;
}
#if DIM <= 3
int intPow(int b, int x){
	int ret = 1;
	for(; x > 0; x--){
		ret *= b;
	}
	return ret;
}
#else
int intPow(int b, int x){//Enable this for higher dimensions (since higher dims tend to call this with larger values). Benchmarked slightly slower at dim 3
	int ret = 1;
	while(x){
		if(x&1){
			ret *= b;
		}
		x >>= 1;
		b *= b;
	}
	return ret;
}
#endif
/*int min(int a, int b){
	if(a < b) return a;
	return b;
}
int max(int a, int b){
	if(a > b) return a;
	return b;
}*/
