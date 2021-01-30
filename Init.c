#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "Global.h"
#include "OClass.h"
extern void generatePointsFromExistsInit();
void inithypercube(){
	#ifndef NDEBUG
		puts("ASSERTION ENABLED");
	#else
		puts("ASSERTION DISABLED");
		assert(0);
	#endif
	//Precompute Tasks
		nCrChooseInit();
		generatePointsFromExistsInit();
	//
}
void cleanuphypercube(){
	deleteAllOClasses();
	deleteHashtable(oclasshandler.map);
	oclasshandler.map = NULL;
}
