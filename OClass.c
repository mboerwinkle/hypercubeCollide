#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "OInstance.h"
#include "OClass.h"
#include "Tree.h"
#include "svg.h"

struct oclasscontainer oclasshandler = {.map = NULL};
void deleteOClassHash(void* key, void* data){
	//key is part of data, so don't free it.
	deleteTree(((oclass*)data)->form);
	free(data);
}
int loadOClass(char* name, char* path){
	FILE* fp = fopen(path, "r");
	if(!fp){
		printf("Failed to load oclass %s\n", path);
		return -1;
	}
	int fDim = 0;
	int mag = 0;
	fscanf(fp, "NHC%d_%X_", &fDim, &mag);
	if(fDim != DIM){//Dimensions don't match
		fclose(fp);
		printf("loaded oclass %s has wrong dimensions\n", path);
		return -1;
	}
	
	oclass* new = malloc(sizeof(oclass));
	strncpy(new->name, name, 21);
	new->name[20] = 0;
	
	//build tree from file.
	new->form = buildTree(fp, mag);
	fclose(fp);
	
	if(verifyTreeMagnitudes(new->form)){
		puts("Tree magnitudes failed!");
	}
	
	if(!(oclasshandler.map)){
		oclasshandler.map = malloc(sizeof(hashtable));
		initHashtable(oclasshandler.map, 2000, &stringHash, &stringCompare, &deleteOClassHash);
	}
	insertHash(oclasshandler.map, new->name, new);
	#ifdef DRAW
		svg* drawOutput = newSvg();
		sprintf(drawOutput->path, "out_%s.svg", new->name);
		treeGenSvg(new->form, drawOutput);
		saveSvg(drawOutput);
		freeSvg(drawOutput);
	#endif
	return 0;
}
void listOClasses(){
	if(!(oclasshandler.map)) return;
	listKeys(oclasshandler.map);
}
void deleteAllOClasses(){
	if(!oclasshandler.map) return;
	deleteAllEntries(oclasshandler.map);
}
int oclassExists(oclass* target, point tloc){
	return 0;
}
oclass* getOClass(char* name){
	oclass* ret = NULL;//TRAINOF THOUGHT RET IS 0. WHY? IS GETDATA BAD?
	getData(oclasshandler.map, name, (void**)(&ret));
	return ret;
}
