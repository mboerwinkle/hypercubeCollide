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
oinstance* newOInstance_common(char type, int colType, point loc){
	assert(type == 's' || type == 'o' || type == 'l');
	oinstance* ret = malloc(sizeof(oinstance));
	assert(ret);
	ret->type = type;
	ret->colType = colType;
	ret->loc = loc;
	return ret;
}
oinstance* newOInstance_o(int colType, point loc, orientation rot, char* classname){
	oinstance* ret = newOInstance_common('o', colType, loc);
	ret->o.rot = rot;
	ret->o.type = getOClass(classname);
	#ifndef NDEBUG
		if(!ret->o.type){
			printf("Unable to retrieve oclass %s\nClasses are: ", classname);
			listKeys(oclasshandler.map);
			assert(0);
		}
	#endif
	//Local Lookup Stack
	ret->o.currTidx = 0;
	ret->o.currT = (tree**)calloc(ret->o.type->form->mag, sizeof(tree*));
	ret->o.currT[0] = ret->o.type->form;
	ret->o.currTloc = (point*)calloc(ret->o.type->form->mag, sizeof(point));
	return ret;
}
oinstance* newOInstance_s(int colType, point loc, int rad){
	oinstance* ret = newOInstance_common('s', colType, loc);
	ret->s.rad = rad;
	return ret;
}
oinstance* newOInstance_l(int colType, point loc, point disp){
	oinstance* ret = newOInstance_common('l', colType, loc);
	ret->l.disp = disp;
	return ret;
}

//FIXME can we specify oinstance ptr as type of a? If so, perform for all hashtable utility functions
void deleteOInstance(void* a, void* b){
	//b is null
	oinstance* o = (oinstance*)a;
	if(o->type == 'o'){
		free(o->o.currT);
		free(o->o.currTloc);
	}
	free(o);
}
//This function does not check surrounding cubes. It just blindreturns the status of the cube at the specified magnitude which holds "inside". Assumes point falls inside definition
/*
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
			//assert(abs(inside[d])-1 <= 1<<(cmag-1));
			if(abs(inside[d])-1 > 1<<(cmag-1)){
				return 'E';
			}
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
	return (curr->full)?'F':'P';
}
*/

char getSpecificCube(oobj* target, int* inside, int mag){//inside is effectively the int array contained in a point.
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
				//assert((31-__builtin_clz(delta)) == (__builtin_clz(delta)^31));
				//outside = (__builtin_clz(delta)^31);
				outside = 1;
				break;
			}
		}
		if(!outside) break;//if we are inside, then continue on to next steps
		if(*currTidx == 0) return 'E';
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
	return (curr->full)?'F':'P';
}

//This is in oinstance instead of oclass for eventual support for instance-specific scaling
char oinstanceExists(oinstance* target, point tloc, int mag){
	for(int d = 0; d < DIM; d++) tloc.p[d] -= target->loc.p[d];//get local displacement
	int sidelen2 = 1<<(mag-1);
	if(target->type == 'o'){
		int formMag = target->o.type->form->mag;
		//when the max mag of instance is bigger than me
		if(mag >= formMag){
			//The tloc/mag doesn't overlap target at all
			if(odistance(tloc) > SQRTDIM * (sidelen2+(1<<(formMag-1)))){
				return 'E';
			}
			return 'P';
		}else{//If this is smaller than the max size of this instance
			tloc = rotatePoint(tloc, target->o.rot, -1.0);//get local reference frame
			char status = getSpecificCube(&(target->o), tloc.p, mag);//This line changes tloc's value
			//printf("smaller %c\n", status);
			if(mag <= RESOLUTION && status != 'E') return 'F';
			return status;
		}
	}else if(target->type == 's'){
		point closest = {.p={0}};
		for(int d = 0; d < DIM; d++){
			if(closest.p[d] < tloc.p[d] - sidelen2){
				closest.p[d] = tloc.p[d]-sidelen2;
			}else if(closest.p[d] > tloc.p[d] + sidelen2){
				closest.p[d] = tloc.p[d]+sidelen2;
			}
		}
		if(odistance(closest) > target->s.rad){
			return 'E';
		}else if(mag <= RESOLUTION) return 'F';
		point furthest;
		for(int d = 0; d < DIM; d++){
			furthest.p[d] = (tloc.p[d] > 0) ? (tloc.p[d] + sidelen2) : (tloc.p[d] - sidelen2);
		}
		if(odistance(furthest) <= target->s.rad) return 'F';
		return 'P';
	}else{
		assert(target->type == 'l');
		point cubecenteredstart, cubecenteredend;
		//used for nDim Cohen-Sutherland. Bits used = 2*DIM
		#if DIM <= 4
		 char
		#else
		 #error unimplemented. Choose some non-char known bit-width integer.
		#endif
		  startbits = 0, endbits = 0;
		int nextcorrectbit = -1;
		for(int d = 0; d < DIM; d++){
			cubecenteredstart.p[d] = -tloc.p[d];
			if(cubecenteredstart.p[d] < -sidelen2){
				startbits |= 1<<(2*d+1);//FIXME remove code duplication and duplicate bitshift calculations
			}else if(cubecenteredstart.p[d] > sidelen2){
				startbits |= 1<<(2*d);
			}
			cubecenteredend.p[d] = cubecenteredstart.p[d]+target->l.disp.p[d];
			if(cubecenteredend.p[d] < -sidelen2){
				nextcorrectbit = 2*d+1;
				endbits |= 1<<nextcorrectbit;//FIXME nextcorrectbit order should be based on accuracy of operation.
			}else if(cubecenteredend.p[d] > sidelen2){
				nextcorrectbit = 2*d;
				endbits |= 1<<nextcorrectbit;
			}
		}
		double enddist = 1.0;
		#ifndef NDEBUG
		 int iterations = 0;
		//int iterationrecord[DIM+1] = {-1};
		#endif
		while(endbits){
			//Per Cohen-Sutherland
			if(startbits & endbits) return 'E';
			#ifndef NDEBUG
			iterations++;
			if(iterations > DIM){
				printf("dist: %lf\n", enddist);
				assert(iterations <= DIM);
			}
			#endif
			//compute target dimension from 'nextcorrectbit'
			int targetdim = nextcorrectbit/2;
			int targetplane = -sidelen2*((nextcorrectbit%2)*2-1);
			enddist = (double) (targetplane-cubecenteredstart.p[targetdim]) / (double) target->l.disp.p[targetdim];
			#ifndef NDEBUG
			if(!(enddist <= 1.0 && enddist >= 0)){
				printf("enddist out of [0,1] range (%lf)\n", enddist);
				printf("targetdim %d targetplane %d relevantcubecenteredstart %d dimdisp %d\n", targetdim, targetplane, cubecenteredstart.p[targetdim], target->l.disp.p[targetdim]);
				printf("endbits: 0x%X nextcorrectbit: %d\n", endbits, nextcorrectbit);
				assert(enddist <= 1.0 && enddist >= 0);
			}
			#endif
			for(int d = 0; d < DIM; d++){
				cubecenteredend.p[d] = cubecenteredstart.p[d] + (int)(target->l.disp.p[d] * enddist);
			}
			assert(cubecenteredend.p[targetdim] == targetplane);
			endbits = 0;
			for(int d = 0; d < DIM; d++){
				if(cubecenteredend.p[d] < -sidelen2){
					nextcorrectbit = 2*d+1;
					endbits |= 1<<nextcorrectbit;//FIXME nextcorrectbit order should be based on accuracy of operation.
				}else if(cubecenteredend.p[d] > sidelen2){
					nextcorrectbit = 2*d;
					endbits |= 1<<nextcorrectbit;
				}
			}
		}
		return (mag <= RESOLUTION) ? 'F' : 'P';
	}
}
