#ifndef OCLASS_H
#define OCLASS_H
#include "Global.h"
#include "Tree.h"
#include "Hashtable.h"


struct oclasscontainer{//singleton
	hashtable* map;
};
extern struct oclasscontainer oclasshandler;

typedef struct oclass{
	char name[21];
	struct tree* form;
}oclass;
extern oclass* getOClass(char* name);
extern int loadOClass(char* name, char* path);
extern void listOClasses();
extern void deleteAllOClasses();
extern int oclassExists(oclass* target, point tloc);
#endif
