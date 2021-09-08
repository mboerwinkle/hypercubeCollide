#ifndef OINSTANCE_H
#define OINSTANCE_H
#include "OClass.h"
#include "Tree.h"



typedef struct osphere{
	int rad;
}osphere;
typedef struct olineseg{
	point disp;
}olineseg;

typedef struct oobj{
	oclass* type;
	orientation rot;

	//These form a stack. This allows for faster access to tree elements, since subsequent accesses are likely to be located in the same subtree (aka physically nearby).
	int currTidx;//where the top of the stack is
	tree** currT;//a stack of our tree location
	point* currTloc;//a stack of coordinates of the centers of tree locations
}oobj;
//Object unions will be represented either as another subtype in oabstr or as a number for 'union code' in oabstr
//Object teams (unions with per-object collision info) need to be represented by a team code number
typedef struct oinstance{
	union{
		osphere s;
		olineseg l;
		oobj o;
	};
	point loc;
	int colType;
	int idx;
	//int minmag;
	char type;
}oinstance;
typedef struct collision{
	oinstance* (p[2]);
}collision;

extern collision* newCollision(oinstance *A, oinstance *B);
extern int hashCollision(void* c);
extern int compareCollision(void* a, void* b);
extern void deleteCollision(void* a, void* b);

extern oinstance* newOInstance_o(int colType, point loc, orientation rot, char* classname);
extern oinstance* newOInstance_s(int colType, point loc, int rad);
extern oinstance* newOInstance_l(int colType, point loc, point disp);
extern void deleteOInstance(void* key, void* data);
extern char oinstanceExists(oinstance* target, point tloc, int mag);
#endif
