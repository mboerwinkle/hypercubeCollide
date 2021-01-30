#ifndef SCENE_H
#define SCENE_H
#include "Global.h"
#include "OInstance.h"
#include "Hashtable.h"
typedef struct sceneQueueObject{
	point loc;
	int mag;
	oinstance** present;
	int presentCount;
	struct sceneQueueObject* next;
}sceneQueueObject;

typedef struct scene{
	int typeCount;//the number of object types (used for collision rules)
	char* cmat;//the matrix of collision rules. Is a symmetric matrix.
	hashtable* object;
	hashtable* collisions;
	struct{
		sceneQueueObject* dequeue;
		sceneQueueObject* enqueue;
	}queue;
}scene;

extern scene* selScene;

extern scene* newScene(int deleteInstances, int typeCount, char* cmat);//deleteInstances set whether you want to delete the instances when the scene is deleted. typecount is the number of object types. cMat is the matrix of collision rules
extern void selectScene(scene* ptr);
extern void freeScene(scene* ptr);
extern int addInstance(oinstance* ptr);
extern void removeInstance(oinstance* ptr);
extern void calculateScene();
extern int* getCollisions();//Gets an int array. first int is the number of collisions. subsequent pairs are indices of objects that collide
#endif
