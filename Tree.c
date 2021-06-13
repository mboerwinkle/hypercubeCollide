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

void calcRelevantCubes(point loc, int mag, int* numPoints){
	assert(mag > 0 && mag < 30);
	point center;
	int sideLen = 1<<mag;
	int sideLen2 = 1<<(mag-1);//why isn't this -2? Because this is used to correct from a corner to a center, not to find child center
	long int radiusSquared = (long int)DIM*sideLen2*sideLen2;//FIXME does not need to be calculated each call
	//long int radius = int_sqrt(radiusSquared);

	for(int d = 0; d < DIM; d++){
		//Find the cube it is a part of, then put it in that cube's center
		center.p[d] = (((loc.p[d])/sideLen)*sideLen) + (loc.p[d] >= 0 ? sideLen2 : -sideLen2);
	}
	long int offset[THREEPOWDIM] = {0};//This is an array of squared offsets from each of the walls
	int middleIdx = THREEPOWDIM/2;
	int idxOffsetToNextMiddle = 1;
	for(int d = 0; d < DIM; d++){//iterate through dimensions, finding single dimension offset status. At or above DIM=4, this is guaranteed for all (because when Dim >= 4, radius >= 2).
		long int walldist1 = sideLen2 - (loc.p[d] - center.p[d]);
		long int walldist2 = sideLen - walldist1;
		offset[middleIdx+idxOffsetToNextMiddle] = walldist1*walldist1;
		offset[middleIdx-idxOffsetToNextMiddle] = walldist2*walldist2;
		idxOffsetToNextMiddle *= 3;
		//starting at the middle of the 3^DIM cube, one face's center can be found by offsetting by 1. The next by offsetting by 3, 9, etc
		/* 2D example
		*  # (1) 2
		* (3) 0 (3)
		*  2 (1) #
		*/
	}
	//Using existing calculated faces, find squared distances to each edge, point, etc.
	#if DIM == 2
		//corners
		offset[0] = offset[1]+offset[3];
		offset[2] = offset[1]+offset[5];
		offset[6] = offset[7]+offset[3];
		offset[8] = offset[7]+offset[5];
	#elif DIM == 3
		//edges
		offset[1] = offset[4]+offset[10];
		offset[3] = offset[4]+offset[12];
		offset[5] = offset[4]+offset[14];
		offset[7] = offset[4]+offset[16];
		offset[9] = offset[10]+offset[12];
		offset[11] = offset[10]+offset[14];
		offset[15] = offset[12]+offset[16];
		offset[17] = offset[14]+offset[16];
		offset[19] = offset[22]+offset[10];
		offset[21] = offset[22]+offset[12];
		offset[23] = offset[22]+offset[14];
		offset[25] = offset[22]+offset[16];
		//corners
		offset[0] = offset[1]+offset[12];
		offset[2] = offset[1]+offset[14];
		offset[6] = offset[7]+offset[12];
		offset[8] = offset[7]+offset[14];
		offset[18] = offset[19]+offset[12];
		offset[20] = offset[19]+offset[14];
		offset[24] = offset[25]+offset[12];
		offset[26] = offset[25]+offset[14];
	#else
		#error undefined
	#endif
	//Create new centers based on the squared distances.
	*numPoints = 0;
	for(int idx = 0; idx < THREEPOWDIM; idx++){
		if(offset[idx] >= radiusSquared) continue;//FIXME check if including everything else in an inverse of this statement is faster than continuing
		point* r = &(ptsFromExists[*numPoints]);
		(*numPoints) += 1;
		(*r) = center;
		for(int d = 0; d < DIM; d++){
			r->p[d] += generatePointsOffsets[idx*DIM+d]*sideLen;//FIXME test if doing center.p[d]+generatePoints... is faster then copying the structure and then adding in the specific elements.
		}
	}
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
