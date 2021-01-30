#ifndef OINSTANCE_H
#define OINSTANCE_H
#include "OClass.h"
#include "Tree.h"
typedef struct oinstance{
	oclass* type;
	int colType;
	point loc;
	orientation rot;
	//TODO double scale;
	int idx;//This is set when the oinstance is put in a scene

	//These form a stack. This allows for faster access to tree elements, since subsequent accesses are likely to be located in the same subtree (aka physically nearby).
	int currTidx;//where the top of the stack is
	tree** currT;//a stack of our tree location
	point* currTloc;//a stack of coordinates of the centers of tree locations
}oinstance;

typedef struct collision{
	oinstance* (p[2]);
}collision;

extern collision* newCollision(oinstance *A, oinstance *B);
extern int hashCollision(void* c);
extern int compareCollision(void* a, void* b);
extern void deleteCollision(void* a, void* b);

extern oinstance* newOInstance(char* classname, int colType, point loc, orientation rot, float scale);
extern void deleteOInstance(void* key, void* data);
extern char oinstanceExists(oinstance* target, point tloc, int mag);
#endif
