#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "Hashtable.h"

int stringHash(void* key){
	int hash = 0;
	int offset = 0;
	int posValue = *(char*)key;
	while(posValue != 0){
		hash+=posValue<<offset;
		offset = (offset+1)%20;//arbitrary
		key++;
		posValue = *(char*)key;
	}
	return hash;
}
int stringCompare(void* key1, void* key2){
	return strcmp(key1, key2);
}

int getHashIdx(hashtable* t, void* key){
	return abs(t->keyHash(key)%t->tableSize);
}

int ptrHash(void* key){
	/*key=(void*)((long int)key/8);//Reduce pointer alignment collisions FIXME bitshift
	int hash = 0;
	for(int x = 0; x < sizeof(void*)/sizeof(int); x++){
		hash ^= ((int*)(&key))[x];
	}*/
	return (long int)key ^ ((long int)key >> (sizeof(void*)-sizeof(int)));
}
int ptrCompare(void* key1, void* key2){
	return key1 != key2;
/*
	if(key1 == key2) return 0;
	return 1;
*/
}
void initHashtable(hashtable* t, int tableSize, int (*keyHash)(void*), int (*keyCompare)(void*, void*), void (*deleteFunc)(void*, void*)){
	t->keyHash = keyHash;
	t->keyCompare = keyCompare;
	t->deleteFunc = deleteFunc;
	t->table = calloc(tableSize, sizeof(hashentry*));
	t->tableSize = tableSize;
	t->entryCount = 0;
	t->first = NULL;
}

void insertHash(hashtable* t, void* key, void* data){
	/*if(getData(t, key, NULL)){
		printf("Cannot insert because key already exists\n");
		return;
	}*/
	int hash = getHashIdx(t, key);
	hashentry* new = malloc(sizeof(hashentry));
	//Set up for linked list function
	new->next = t->first;
	t->first = new;
	new->prev = NULL;
	if(new->next != NULL) new->next->prev = new;
	//transfer info
	new->key = key;
	new->data = data;
	//add into chaining table.
	new->follow = t->table[hash];
	t->table[hash] = new;
	t->entryCount++;
}
int getData(hashtable* t, void* key, void** data){
	int hash = getHashIdx(t, key);
	hashentry* test = t->table[hash];
	while(test != NULL){
		if(t->keyCompare(key, test->key) == 0){
			if(data != NULL) *data = test->data;
			return 1;
		}
		test = test->follow;
	}
	return 0;
}
void deleteEntry(hashtable* t, void* key){
	hashentry** bucket = &(t->table[getHashIdx(t, key)]);//the pointer to the pointer to the first item in that bucket
	hashentry* target = *bucket;//the pointer to the first item in the bucket
	hashentry** ptr = bucket;//the pointer to the pointer that needs to be changed once the target is deleted.
	while(target != NULL && t->keyCompare(target->key, key)){
		ptr = &(target->follow);
		target = target->follow;
	}
	if(target == NULL){
		puts("Could not delete entry");
		return;
	}
	//Fix the linked list stuff.
	if(target->next != NULL) target->next->prev = target->prev;
	if(target->prev != NULL){
		target->prev->next = target->next;
	}else{//if there isnt a previous, then we must be first, so we have to redirect first.
		t->first = target->next;
	}
	//Fix the chaining.
	*ptr = target->follow;
	//delete the data/key
	if(t->deleteFunc != NULL) t->deleteFunc(target->key, target->data);
	free(target);
	t->entryCount--;
}
void deleteAllEntries(hashtable* t){
	hashentry *next, *cond = t->first;//cond is 'conductor'
	while(cond != NULL){
		next = cond->next;
		if(t->deleteFunc != NULL) t->deleteFunc(cond->key, cond->data);
		free(cond);
		t->entryCount--;
		cond = next;
	}
	t->first = NULL;
	assert(t->entryCount == 0);
	memset(t->table, 0, t->tableSize*sizeof(hashentry*));
}
void printTableStatistics(hashtable* t){//FIXME sum of square difference from median fill
	int used = 0;
	for(int idx = 0; idx < t->tableSize; idx++){
		if(t->table[idx] != NULL){
			used++;
		}
	}
	double loadFactor = (double)used/(double)(t->tableSize);
	double idealLoadFactor = (double)t->entryCount/(double)(t->tableSize);
	if(idealLoadFactor > 1.0) idealLoadFactor = 1.0;
	double avgPerBucket = t->entryCount/(double) used;
	printf("Table Size: %d\tItems: %d\tLoad Factor: %.2lf (%.2lf ideal)\tAverage Per: %.2lf\n",t->tableSize, t->entryCount, loadFactor, idealLoadFactor, avgPerBucket);
}
void printAllHashes(hashtable* t){
	printf("PRINTING ALL HASHES:\n");
	hashentry* current = t->first;
	while(current != NULL){
		int hash = t->keyHash(current->key);
		printf("%d\n", hash);
		current = current->next;
	}
	printf("DONE\n");
}
void deleteHashtable(hashtable* t){
	if(t == NULL) return;
	deleteAllEntries(t);
	free(t->table);
	free(t);
}

void listKeys(hashtable* t){
	hashentry* cond = t->first;
	while(cond != NULL){
		printf("%20s  ", (char*)(cond->key));
		cond = cond->next;
	}
	printf("\n");
}
