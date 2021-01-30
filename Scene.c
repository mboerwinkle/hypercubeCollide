#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "Scene.h"
#include "Global.h"

scene* selScene = NULL;

scene* newScene(int deleteInstances, int typeCount, char* cmat){
	scene* ret = malloc(sizeof(scene));
	ret->typeCount = typeCount;
	ret->cmat = malloc(typeCount*typeCount);//Allocate array for collision rules
	//memcpy(ret->cmat, cmat, typeCount*typeCount);
	for(int cmatIdx = 0; cmatIdx < typeCount*typeCount; cmatIdx++){
		if(cmat[cmatIdx] == '1'){
			ret->cmat[cmatIdx] = 1;
		}else if(cmat[cmatIdx] == '0'){
			ret->cmat[cmatIdx] = 0;
		}else{
			printf("FATAL: unknown character in cmat string: \"%10s\"\n", cmat);
		}
	}
	void (*deleteFunc)(void*, void*);
	if(deleteInstances){
		deleteFunc = &deleteOInstance;
	}else{
		deleteFunc = NULL;
	}
	ret->object = malloc(sizeof(hashtable));
	initHashtable(ret->object, OINSTANCEHASHSIZE, &ptrHash, &ptrCompare, deleteFunc);
	ret->collisions = malloc(sizeof(hashtable));
	initHashtable(ret->collisions, COLLISIONHASHSIZE, &hashCollision, &compareCollision, &deleteCollision);//FIXME collisions should really be a bitmap for their usage, but then I'd have to make a bitmap of pointers or a direct access interface to the hashtable or not store objects in a hashtable...
	ret->queue.enqueue = NULL;
	ret->queue.dequeue = NULL;
	return ret;
}
void selectScene(scene* ptr){
	selScene = ptr;
}
void freeScene(scene* ptr){
	deleteHashtable(ptr->object);
	deleteHashtable(ptr->collisions);
	free(ptr->cmat);
	free(ptr);
	if(selScene == ptr) selScene = NULL;
}
int addInstance(oinstance* ptr){
	ptr->idx = selScene->object->entryCount;
	insertHash(selScene->object, ptr, NULL);
	return ptr->idx;//Return the index of this object for this scene
}
int* getCollisions(){
	int collisions = selScene->collisions->entryCount;
	int len = 1 + collisions*2;
	int* ret = malloc(sizeof(int) * len);
	ret[0] = collisions;
	hashentry* curr = selScene->collisions->first;
	int cIdx = 0;
	while(curr != NULL){
		oinstance** col = ((collision*)curr->key)->p;
		//Get the idx of the collisions (+1 to offset for ret[0] = len, +2 for the next one.)
		ret[1 + 2*cIdx] = col[0]->idx;
		ret[2 + 2*cIdx] = col[1]->idx;
		cIdx++;
		curr = curr->next;
	}
	//puts("Collision hashtable:");
	//printTableStatistics(selScene->collisions);
	return ret;
}
