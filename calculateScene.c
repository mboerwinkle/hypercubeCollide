#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "Scene.h"
#include "svg.h"

#ifdef DRAW
	svg* draw;
#endif

point sceneCenter;
point sceneMax;
point sceneMin;
int sceneMagnitude = 0;

void calculateSceneLocus();
void initSceneQueue();
void calculateNextSceneQueueObject();
sceneQueueObject* createSceneQueueObject(point loc, int mag, int presentCount);
void deleteSceneQueueObject(sceneQueueObject* t);
void addToQueue(sceneQueueObject* t);
point getChildCenter(point ploc, int pmag, int cIdx);
void calculateScene(){
	#ifdef DRAW
		draw = newSvg();
		sprintf(draw->path, "out.svg");
	#endif
	deleteAllEntries(selScene->collisions);
	calculateSceneLocus();
	#ifdef DRAW
		addSvgObj(draw, newSvgRect(sceneMin.p[0], sceneMin.p[1], sceneMax.p[0], sceneMax.p[1], gray, 1, 1000));
	#endif
	initSceneQueue();
	int sceneQueueTotalCount = 0;
	while(selScene->queue.dequeue != NULL){
		calculateNextSceneQueueObject();
		sceneQueueTotalCount++;
	}
	//printf("%d entries\n", sceneQueueTotalCount);
	#ifdef DRAW
		saveSvg(draw);
		freeSvg(draw);
	#endif
}

void calculateSceneLocus(){
	for(int d = 0; d < DIM; d++){
		sceneCenter.p[d] = 0;
		sceneMax.p[d] = POINTMIN;
		sceneMin.p[d] = POINTMAX;
	}
	//currHash is the chained hash entry for the object
	hashentry *currHash = selScene->object->first;
	while(currHash != NULL){
		//curr is the pointer to the current object
		oinstance *curr = currHash->key;
		//calculate diameter of instance (the distance to the corner)
		int radius;
		if(curr->type == 'o'){
			radius = (1<<(curr->o.type->form->mag-1))*SQRTDIM+1;
		}else if(curr->type == 's'){
			radius = curr->s.rad;
		}else{
			assert(curr->type == 'l');
			radius = odistance(curr->l.disp);
		}
		#ifdef DRAW
			addSvgObj(draw, newSvgCircle(curr->loc.p[0], curr->loc.p[1], radius, green, 0, 250));
		#endif
		for(int d = 0; d < DIM; d++){
			int min = curr->loc.p[d]-radius;
			int max = curr->loc.p[d]+radius;
			if(min < sceneMin.p[d]) sceneMin.p[d] = min;
			if(max > sceneMax.p[d]) sceneMax.p[d] = max;
		}
		currHash = currHash->next;
	}
	int maxOffset = 0;
	for(int d = 0; d < DIM; d++){
		sceneCenter.p[d] = (sceneMin.p[d]+sceneMax.p[d])/2;
		//Calculate maxoffset
		int testOffset = abs(sceneMin.p[d]-sceneCenter.p[d]);
		if(testOffset > maxOffset) maxOffset = testOffset;
		testOffset = abs(sceneMax.p[d]-sceneCenter.p[d]);
		if(testOffset > maxOffset) maxOffset = testOffset;
	}
	/*printf("Axis\tMin\tMax\tCenter\n");
	for(int d = 0; d < DIM; d++){
		printf("%d\t%d\t%d\t%d\n", d, sceneMin.p[d], sceneMax.p[d], sceneCenter.p[d]);
	}*/
	sceneMagnitude = 0;
	while(1<<sceneMagnitude < maxOffset*2){
		sceneMagnitude++;
	}
	//printf("Max Cube Side Len %d\tMagnitude %d\n", maxOffset*2, sceneMagnitude);
}
void initSceneQueue(){
	sceneQueueObject* initq = createSceneQueueObject(sceneCenter, sceneMagnitude, selScene->object->entryCount);
	hashentry *currHash = selScene->object->first;
	int idx = 0;
	while(currHash != NULL){
		initq->present[idx] = (oinstance*)(currHash->key);
		idx++;
		#ifdef DRAW
			#error needs rework for oinstance split
			char name[30];
			sprintf(name, "out_%s.svg", ((oinstance*)(currHash->key))->type->name);
			int sideLen2 = 1<<(((oinstance*)(currHash->key))->type->form->mag-1);
			addSvgObj(draw, newSvgRef(name, ((oabstr*)(currHash->key))->loc.p[0]-sideLen2, ((oabstr*)(currHash->key))->loc.p[1]-sideLen2, sideLen2*2, sideLen2*2, -((oinstance*)(currHash->key))->rot.r, black, 0));
		#endif
		currHash = currHash->next;
	}
	assert(idx == initq->presentCount);
	addToQueue(initq);
}
void calculateNextSceneQueueObject(){
	sceneQueueObject* t = selScene->queue.dequeue;
	selScene->queue.dequeue = t->next;
	if(t->next == NULL){
		assert(selScene->queue.enqueue == t);
		selScene->queue.enqueue = NULL;
	}

	//printf("Processing queue object. Mag: %d present: %d\n", t->mag, t->presentCount);

	#ifdef DRAW
		addSvgObj(draw, newSvgRect(t->loc.p[0]-(1<<(t->mag-1)), t->loc.p[1]-(1<<(t->mag-1)), t->loc.p[0]+(1<<(t->mag-1)), t->loc.p[1]+(1<<(t->mag-1)), red, 0, 500));
		//printf("Scene Queue: %d %d Mag %d\n", t->loc.p[0], t->loc.p[1], t->mag);
	#endif
	//TODO: We can removal pool before and after.(currently just before) Very likely one is beneficial, but not both. TEST.
	
	//Create removal pool
	//For each element, check against every element in _full_pool
		//if it doesnt match, remove it and its (lack of) match from_removal_pool
	//remove every element in removal pool
	{
	char removeList[t->presentCount];//if it is zero, remove it. we set both to 1 when they don't collide (but can collide).
	memset(removeList, 0, t->presentCount);
	collision test;
	for(int idx = 0; idx < t->presentCount; idx++){
		if(removeList[idx]) continue;//if already known to stay, no need to check.
		test.p[0] = t->present[idx];
		char c1 = test.p[0]->colType;//c1 and c2 are colType: the indices for the collision matrix
		for(int idx2 = 0; idx2 < t->presentCount; idx2++){//TODOmake count backwards as optimization?
			if(idx == idx2) continue;
			test.p[1] = t->present[idx2];
			char c2 = test.p[1]->colType;
			if(selScene->cmat[c1*selScene->typeCount+c2] && !(getData(selScene->collisions, &test, NULL))){//these can collide AND these don't already collide
				removeList[idx] = 1;//this one needs to stay.
				removeList[idx2] = 1;//this one does too.
				break;
			}
		}
	}
	int tailIdx = t->presentCount-1;
	for(int idx = 0; idx < tailIdx; idx++){//in place sort of 1's and 0's only in linear time.
		while(!removeList[idx] && idx < tailIdx){
			removeList[idx] = removeList[tailIdx];
			t->present[idx] = t->present[tailIdx];
			tailIdx--;
		}
	}
	if(removeList[tailIdx]){
		t->presentCount = tailIdx+1;//tailidx is last 1 (the join point of the sort says 'keep')
	}else{
		t->presentCount = tailIdx;//tailIdx is first 0 (the join point of the sort says 'leave it')
	}
	//printf("present %d\n", t->presentCount);
	}//end of first removal pool
	//For each element
		//if full, collides with all other members (At the end! not empty ones!)
		//Partial, add to all subqueues
		//Empty, remove from list

	oinstance* (fulls[t->presentCount]);//These are present indices which are full (avoids iterating through every object looking for fulls)
	int fullCount = 0;
	oinstance* (partials[t->presentCount]);//These are present indices which are partial (avoids iterating and checking multiple times)
	int partialCount = 0;

	for(int idx = 0; idx < t->presentCount; idx++){//get statuses and remove empties.
		char result = oinstanceExists(t->present[idx], t->loc, t->mag);
		if(result == 'P'){
			partials[partialCount] = t->present[idx];
			partialCount++;
		}else if(result == 'F'){
			fulls[fullCount] = t->present[idx];
			fullCount++;
		}
	}
	collision test;
	for(int idx = 0; idx < fullCount; idx++){
		test.p[0] = fulls[idx];
		for(int idx2 = idx+1; idx2 < fullCount; idx2++){//Create collisions between all fulls and all other fulls.
			test.p[1] = fulls[idx2];
			if(!getData(selScene->collisions, &test, NULL)){//if this collision doesn't exist
				insertHash(selScene->collisions, newCollision(test.p[0], test.p[1]), NULL);
			}
		}
		for(int idx2 = 0; idx2 < partialCount; idx2++){//Create collisions between all fulls and all partials.
			test.p[1] = partials[idx2];
			if(!getData(selScene->collisions, &test, NULL)){//if this collision doesn't exist
				insertHash(selScene->collisions, newCollision(test.p[0], test.p[1]), NULL);
			}
		}
	}
	if(partialCount > 1){
		for(int idx = 0; idx < TWOPOWDIM; idx++){
			sceneQueueObject* child = createSceneQueueObject(getChildCenter(t->loc, t->mag, idx), t->mag-1, partialCount);
			memcpy(child->present, partials, partialCount*sizeof(oinstance*));//copy our list of children to each of the children.
			addToQueue(child);
		}
	}
	//printf("%d children have %d members\n", t->mag-1, children[0]->presentCount);
	//Create removal pool(all elements that were partial)
	//For each element, check against every element in _full_pool
		//if it doesnt match, remove it and its (lack of) match from_removal_pool
	//remove every element in removal pool
	deleteSceneQueueObject(t);
}
void deleteSceneQueueObject(sceneQueueObject* t){
	free(t->present);
	free(t);
}
void addToQueue(sceneQueueObject* t){
	if(selScene->queue.enqueue != NULL){
		assert(selScene->queue.dequeue != NULL);
		selScene->queue.enqueue->next = t;
	}else{
		assert(selScene->queue.dequeue == NULL);
		selScene->queue.dequeue = t;
	}
	selScene->queue.enqueue = t;
}
sceneQueueObject* createSceneQueueObject(point loc, int mag, int presentCount){
	sceneQueueObject* ret = malloc(sizeof(sceneQueueObject));
	ret->loc = loc;
	ret->mag = mag;
	ret->present = malloc(presentCount * sizeof(oinstance*));
	ret->presentCount = presentCount;
	ret->next = NULL;
	return ret;
}
point getChildCenter(point ploc, int pmag, int cIdx){
	int offset = 1<<(pmag-2);//pmag is complete field dimension. pmag-1 is from center to wall. pmag-2 is from center to child center.
	point ret = ploc;
	for(int d = 0; d < DIM; d++){
		if((cIdx>>d) & 1){
			ret.p[d]+=offset;
		}else{
			ret.p[d]-=offset;
		}
	}
	return ret;
}
