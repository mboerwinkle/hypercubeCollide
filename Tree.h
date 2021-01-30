#ifndef TREE_H
#define TREE_H
#include "Global.h"
#include "svg.h"
typedef struct tree{
	struct tree* (c[TWOPOWDIM]);
	int mag;
	char full;//1 or 0;
}tree;
extern void deleteTree(tree* t);
extern tree* buildTree(FILE* fp);//mag is the magnitude of the tree. mag is the max+1
extern int verifyTreeMagnitudes(tree* t);
//This function finds the cube overlapped by the given cube. In the future, it should accept rotation to further narrow it down.
extern point ptsFromExists[THREEPOWDIM];
extern void calcRelevantCubes(point loc, int mag, int* numPoints);
extern void treeGenSvg(tree* t, svg* s);
#endif
