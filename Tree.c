#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <math.h>//FIXME remove
#include <assert.h>
#include "Tree.h"
void deleteTree(tree* t){
	for(int x = 0; x < TWOPOWDIM; x++){
		if(t->c[x] != NULL) deleteTree(t->c[x]);
	}
	free(t);
}

void fixTreeMag(tree* t, int mag);

tree* buildTree(FILE* fp){
	char c = fgetc(fp);
	if(c == 'E'){
		return NULL;
	}
	tree* ret = malloc(sizeof(tree));
	if(c == 'P'){
		ret->full = 0;
		int maxMag = -1;
		for(int x = 0; x < TWOPOWDIM; x++){
			ret->c[x] = buildTree(fp);
			if(ret->c[x]){
				int testMag = ret->c[x]->mag;
				if(testMag > maxMag) maxMag = testMag;
			}
		}
		assert(maxMag >= 1);
		fixTreeMag(ret, maxMag+1);
	}else if(c == 'F'){
		ret->full = 1;
		ret->mag = 1;
		for(int x = 0; x < TWOPOWDIM; x++){
			ret->c[x] = NULL;
		}
		//bzero(ret->c, TWOPOWDIM*sizeof(tree*));
	}else{
		printf("Buildtree encountered unknown character \"%c\"", c);
		free(ret);
		ret = NULL;
	}
	return ret;
}

void fixTreeMag(tree* t, int mag){
	t->mag = mag;
	for(int x = 0; x < TWOPOWDIM; x++){
		if(t->c[x] != NULL && t->c[x]->mag != mag-1) fixTreeMag(t->c[x], mag-1);
	}
}
int verifyTreeMagnitudes(tree* t){
	int mag = t->mag;
	int ret = 0;
	for(int idx = 0; idx < TWOPOWDIM; idx++){
		if(t->c[idx] == NULL) continue;
		if(t->c[idx]->mag != mag-1) ret++;
		ret+=verifyTreeMagnitudes(t->c[idx]);
	}
	return ret;
}
int progressNextElement(int* t, int len){//This function takes an array of -1's, 0's, and 1's, and changes all 1's to -1's to get all combos (leaving 0's alone) to get all possible elements. returns 1 if ends on not all 1's, returns 0 if all 1's.
	int idx = 0;
	//printf("progressNextElement\n");
	//for(int x = 0; x < len; x++) printf("%d ", t[x]);
	//printf("\n");
	while(1){//This approach is surprisingly slightly faster than a for-loop based one.
		while(idx < len && t[idx] == 0){
			idx++;
		}
		if(idx >= len){
			return 0;
		}
		assert(idx < len);
		if(t[idx] == 1){
			t[idx] = -1;
			return 1;
		}else{
			t[idx] = 1;
			idx++;
		}
	}
	//for(int x = 0; x < len; x++) printf("%d ", t[x]);
	//printf("\n");
	//printf("RETURN: %d\n", ret);
}

/*e-xists
w-hich
*/
void setExistsFromWhich(char* e, int* w){
#if DIM == 2
	e[4] = 1;
	e[(w[0]+1)*3+1] = 1;
	e[3+w[1]+1] = 1;
	e[(w[0]+1)*3+w[1]+1] = 1;
#elif DIM == 3
	e[13]								= 1;//13 is the exact middle of [0-26]. The center cube always exists.
	e[(w[0]+1)*9+3+1]					= 1;
	e[9+(w[1]+1)*3+1]					= 1;
	e[(w[0]+1)*9+(w[1]+1)*3+1]			= 1;
	e[9+3+(w[2]+1)]						= 1;
	e[(w[0]+1)*9+3+(w[2]+1)]			= 1;
	e[9+(w[1]+1)*3+(w[2]+1)]			= 1;
	e[(w[0]+1)*9+(w[1]+1)*3+(w[2]+1)]	= 1;
#else
	#error not defined
#endif
}
//This defines the offsets of every adjacent cube around a central cube in DIM
char generatePointsOffsets[DIM*THREEPOWDIM];
void generatePointsFromExistsInit(){
	puts("## generatePointsFromExists");
	for(int idx = 0; idx < THREEPOWDIM; idx++){
		printf("{");
		for(int d = 0; d < DIM; d++){
			generatePointsOffsets[idx*DIM+d] = ((idx/intPow(3, DIM-(d+1)))%3)-1;//resultant offset is one of -1, 0, 1 in all dimensions
			if(d != 0) printf(", ");
			printf("%d", generatePointsOffsets[idx*DIM+d]);
		}
		printf("}\n");
	}
	puts("##");
}
point ptsFromExists[THREEPOWDIM];
void calcPointsFromExists(int* numPoints, void* exists, point center, int sideLen){//put this in calcRelevantCubes
	*numPoints = 0;
	for(int idx = 0; idx < THREEPOWDIM; idx++){
		if(((char*) exists)[idx] == 0) continue;//FIXME check if including everything else in an inverse of this statement is faster than continuing
		point* r = &(ptsFromExists[(*numPoints)]);
		(*r) = center;
		for(int d = 0; d < DIM; d++){
			r->p[d] += generatePointsOffsets[idx*DIM+d]*sideLen;//FIXME test if doing center.p[d]+generatePoints... is faster then copying the structure and then adding in the specific elements.
		}
		(*numPoints)++;
	}
}

void calcRelevantCubes(point loc, int mag, int* numPoints){
	point center;
	int sideLen = 1<<mag;
	int sideLen2 = 1<<(mag-1);//why isn't this -2? Because this is used to correct from a corner to a center, not to find child center
	long int radiusSquared = sideLen2*sideLen2*DIM;
	for(int d = 0; d < DIM; d++){
		//Find the cube it is a part of, then put it in that cube's center
		center.p[d] = (((loc.p[d])/sideLen)*sideLen) + (loc.p[d] >= 0 ? sideLen2 : -sideLen2);
		/*if(loc.p[d] >= 0){
			center.p[d]+=sideLen2;
		}else{
			center.p[d]-=sideLen2;
		}*/
		//printf(" %d", center.p[d]);
	}
	//printf("\n");
	char exists[THREEPOWDIM] = {0};//This is an array of chars equal to 3^dim (aka 9 for dim2, 27 for dim3)
	//FIXME make fewer dimensions actually be called recursively if the higher dimension ones don't work.
	int which[DIM];//For 3D, {1,0,-1} is a line along y at positive x and negative z.
	for(int d = DIM; d >= 1; d--){//This is the number of dimensions we are combining. We start high to eliminate redundant work (e.g. a corner eliminates faces and edges, but nothing can eliminate a point. A point is maximum combined dimensions.
		for(int e = 0; e < nCr[DIM][d]; e++){
			nCrChoose(DIM, d, e, which);//Figure out which dimensions we are combining
			do{//Get all positive and negative options
				point projection;
				for(int d2 = 0; d2 < DIM; d2++){//This for loop used to be a function "createProjectionPoint"
					projection.p[d2] = center.p[d2]+which[d2]*sideLen2;
				}
				//Distancesquared(loc, projection)
				/*double distSquared = 0.0;
				for(int distSqDim = 0; distSqDim < DIM; distSqDim++){
					distSquared+=(loc.p[distSqDim]-projection.p[distSqDim])*(loc.p[distSqDim]-projection.p[distSqDim]);
				}*/
				#if DIM == 2
					long int distSquared = (loc.p[0]-projection.p[0])*(loc.p[0]-projection.p[0])+(loc.p[1]-projection.p[1])*(loc.p[1]-projection.p[1]);
				#elif DIM == 3
					long int distSquared = (loc.p[0]-projection.p[0])*(loc.p[0]-projection.p[0])+(loc.p[1]-projection.p[1])*(loc.p[1]-projection.p[1])+(loc.p[2]-projection.p[2])*(loc.p[2]-projection.p[2]);
				#else
					#error undefined
				#endif
				//end distancesquared
				//if(radiusSquared > distanceSquared(loc, projection)){
				if(radiusSquared > distSquared){
					setExistsFromWhich(exists, which);
				}
			}while(progressNextElement(which, DIM));
		}
	}
	calcPointsFromExists(numPoints, exists, center, sideLen);
}

void treeGenSvgRec(tree* t, svg* s, point p){
	int sideLen2 = 1<<(t->mag-1);
	if(t->full){
		addSvgObj(s, newSvgRect(p.p[0]-sideLen2, p.p[1]-sideLen2, p.p[0]+sideLen2, p.p[1]+sideLen2, black, 0, 0));
		return;
	}
	int sideLen4 = sideLen2/2;
	for(int y = 0; y < 2; y++){
		for(int x = 0; x < 2; x++){
			int cIdx = y+2*x;
			if(t->c[cIdx] == NULL) continue;
			point cp = p;
			if(x){
				cp.p[0]+=sideLen4;
			}else{
				cp.p[0]-=sideLen4;
			}
			if(y){
				cp.p[1]+=sideLen4;
			}else{
				cp.p[1]-=sideLen4;
			}
			treeGenSvgRec(t->c[cIdx], s, cp);
		}
	}
}
void treeGenSvg(tree* t, svg* s){
	point center = {.p = {0, 0}};
	double sideLen2 = 1<<(t->mag-1);
	addSvgObj(s, newSvgRect(-sideLen2, -sideLen2, sideLen2, sideLen2, yellow, 0, 0));
	treeGenSvgRec(t, s, center);
}
