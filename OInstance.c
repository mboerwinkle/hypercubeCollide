#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "Hashtable.h"
#include "OInstance.h"

#ifdef DRAW
	extern svg* draw;
#endif
collision* newCollision(oinstance *A, oinstance *B){
	collision* ret = malloc(sizeof(collision));
	ret->p[0] = A;
	ret->p[1] = B;
	return ret;
}
//This function should return the same for flipped collisions too
int hashCollision(void* c){
	collision* t = c;
	return ptrHash(t->p[0]) + ptrHash(t->p[1]);
}
int compareCollision(void* a, void* b){
	collision *A = a, *B = b;
	oinstance *a1 = A->p[0], *b1 = A->p[1], *a2 = B->p[0], *b2 = B->p[1];
	return !((a1 == a2 && b1 == b2) || (a1 == b2 && b1 == a2));
}
void deleteCollision(void* a, void* b){
	//a is the collision. b doesn't exist.
	free(a);
}
oinstance* newOInstance(char* classname, int colType, point loc, orientation rot, float scale){
	oinstance* ret = malloc(sizeof(oinstance));
	ret->type = getOClass(classname);
	if(ret->type == NULL){
		printf("Unable to retrieve oclass %s\nClasses are: ", classname);
		listKeys(oclasshandler.map);
	}
	//coltype is used for collisionmaps
	ret->colType = colType;
	ret->loc = loc;
	ret->rot = rot;
//	ret->scale = scale;

	//fast lookup stack
	ret->currTidx = 0;
	ret->currT = (tree**)calloc(ret->type->form->mag, sizeof(tree*));
	ret->currT[0] = ret->type->form;
	ret->currTloc = (point*)calloc(ret->type->form->mag, sizeof(point));
	//calloc zeros, so currTloc[0] is correctly all 0.
	//end fast lookup stack
	return ret;
}
void deleteOInstance(void* a, void* b){
	//b is null
	oinstance* o = (oinstance*)a;
	free(o->currT);
	free(o->currTloc);
	free(o);
}
/*//This function does not check surrounding cubes. It just blindreturns the status of the cube at the specified magnitude which holds "inside". Assumes point falls inside definition
char getSpecificCube(oinstance* target, int* inside, int mag){//inside is effectively the int array contained in a point.
	//printf("%X %d %d\n", (unsigned int)((long int)target&0xFFF), inside[0], inside[1]);
	tree *curr = target->type->form;
	int cmag = curr->mag;
	while(cmag != mag){
	//while(curr->mag != mag){
		assert(cmag >= RESOLUTION);
		if(curr->full) return 'F';
		int idx = 0;
		for(int d = 0; d < DIM; d++){//This block determines the child idx and the local coordinates for the point in the child.
			if(inside[d] >= 0){
				idx |= (1<<(DIM-1-d));//Here we are setting bits of the child index. dim 0(x) sets bit 3-1-0 = 2 (4b10 bit), while dim 2(z) sets 3-1-2 = 0th bit
				//idx |= (1<<d);//FIXME Faster?
				inside[d] -= 1<<(cmag-2);
			}else{
				inside[d] += 1<<(cmag-2);
			}
		}
		if(curr->c[idx] != NULL){//FIXME optimize to cache pointer
			curr = curr->c[idx];
			cmag--;
		}else{
			return 'E';
		}
	}
	if(curr->full){
		return 'F';
	}else{
		return 'P';
	}
}*/

char getSpecificCube(oinstance* target, int* inside, int mag){//inside is effectively the int array contained in a point.
	tree** currT = target->currT;
	point* currTloc = target->currTloc;
	int* currTidx = &target->currTidx;

	tree *curr = currT[*currTidx];
	int cmag = curr->mag;
	if(cmag < mag){//if we are currently lower than we need to be, go up.
		puts("lower");
		int delta = mag - cmag;
		*currTidx -= delta;
		curr = currT[*currTidx];
		cmag += delta;
		assert(cmag == mag);
	}
	int* center = currTloc[*currTidx].p;
	while(1){//while the target isn't inside us, go up. FIXME can we underestimate how much we need to go up by looking at the power of the distance?
		int maxdist = 1<<(cmag-1);
		int outside = 0;
		for(int d = 0; d < DIM; d++){
			int delta = abs(inside[d]-center[d]);
			if(delta > maxdist){
				/*
				assert((31-__builtin_clz(delta)) == (__builtin_clz(delta)^31));
				outside = (__builtin_clz(delta)^31);*/
				outside = 1;
				break;
			}
		}
		if(!outside) break;//if we are inside, then continue on to next steps
		(*currTidx)--;
		//(*currTidx) = max((*currTidx)-1, outside);
		curr = currT[*currTidx];
		cmag++;
		center = currTloc[*currTidx].p;
	}
	int* childcenter;
	while(cmag != mag){//while we aren't at the right height, go down.
		assert(cmag >= RESOLUTION);
		if(curr->full) return 'F';
		int idx = 0;
		childcenter = currTloc[(*currTidx) + 1].p;
		for(int d = 0; d < DIM; d++){//This block determines the child idx and the local coordinates for the point in the child.
			if(inside[d] >= center[d]){
				idx |= (1<<(DIM-1-d));//Here we are setting bits of the child index. dim 0(x) sets bit 3-1-0 = 2 (4b10 bit), while dim 2(z) sets 3-1-2 = 0th bit
				childcenter[d] = center[d]+(1<<(cmag-2));
			}else{
				childcenter[d] = center[d]-(1<<(cmag-2));
			}
		}
		if(curr->c[idx] != NULL){
			curr = curr->c[idx];
			(*currTidx)++;
			currT[*currTidx] = curr;
			center = childcenter;
			cmag--;
		}else{
			return 'E';
		}
	}
	if(curr->full){
		return 'F';
	}else{
		return 'P';
	}
}

//This is in oinstance instead of oclass for eventual support for instance-specific scaling
char oinstanceExists(oinstance* target, point tloc, int mag){
	for(int d = 0; d < DIM; d++) tloc.p[d] -= target->loc.p[d];//get local displacement
	tloc = rotatePoint(tloc, target->rot, -1.0);//get local reference frame
	int relevantCount = 0;
	calcRelevantCubes(tloc, mag, &relevantCount);//FIXME getRelevantCubes gets called on mag=0, which it hates.
	assert(relevantCount > 0);
	char full = 1;//FIXME make bit fields of single char to allow for efficient early exit in later parts of algo.
	char empty = 1;
	int formMag = target->type->form->mag;
	int* relpt;
	if(mag >= formMag){//This block of code just handles points which are out of bounds for the instance.
		for(int t = 0; t < relevantCount; t++){
			relpt = ptsFromExists[t].p;
			char implicitExternal = 0;
			for(int d = 0; d < DIM; d++){
				if(abs(relpt[d]) > 1<<(mag-1)){//The target is too small to notice, but outside //FIXME 1<<(mag-1) gets computed an awful lot.
					implicitExternal = 1;
					break;
				}
			}
			if(implicitExternal){
				//if(xtreme) puts("Implicit External");
				full = 0;//It is too small to notice. Luckily, it is outside.
				continue;
			}else{
				//if(xtreme) puts("Implicit Parent");
				return 'P';
			}
		}
	}else{//If this is smaller than the max size of this instance
		for(int t = 0; t < relevantCount; t++){
			relpt = ptsFromExists[t].p;
			char outside = 0;
			for(int d = 0; d < DIM; d++){//FIXME - this is expensive. Check initially if it is possibly applicable to any of them to hopefully avoid it altogether.
				if(abs(relpt[d]) > 1<<(formMag-1)){//FIXME 1<<(formMag-1) gets computed an awful lot.
					outside = 1;
					break;
				}
			}
			if(outside){
				full = 0;
				continue;
			}
			/*int sideLen2 = (1<<(target->type->form->mag-1));//Sidelen/2 of the model. This is the limit where it becomes 'P', and doesn't check.
			int maxDist = sideLen2+(1<<(mag-1));//Maxdist is the sideLen/2 of the model, plus the sideLen/2 of the target cube. This is the limit where it becomes 'E'
			int outside = 0, farOutside = 0;//Outside is if the relevant cube is outside the model, but it overlaps. farOutside is if it doesn't touch at all.//FIXME verify all options/checks are used
			for(int d = 0; d < DIM; d++){
				if(sideLen2 < abs(relevant[t].p[d])){
					outside = 1;
					if(maxDist <= abs(relevant[t].p[d])){
						farOutside = 1;
					}
				}
			}
			if(outside & !farOutside){
				free(relevant);
				if(xtreme) puts("Outside, but not far outside");
				return 'P';
			}
			if(farOutside){
				full = 0;
				if(xtreme) puts("Far outside.");
				continue;
			}*/
			//End block of code
			char status = getSpecificCube(target, relpt, mag);//This line changes relpt value.
			if(status == 'P'){
				assert(mag != 1);
				return 'P';
			}else if(status == 'F'){
				empty = 0;
			}else{//Must be 'E'
				full = 0;
			}
		}
	}
	assert(full+empty != 2);
	if(empty) return 'E';
	if(full || mag <= RESOLUTION) return 'F';
	return 'P';
}
