#ifndef HASHTABLE_H
#define HASHTABLE_H

typedef struct hashentry{
	void* key;
	void* data;
	struct hashentry* follow;//my followers (chaining)
	struct hashentry* next;//used for linked list functions.
	struct hashentry* prev;//also used for linked list. required for individual deletion to repair the chain.
}hashentry;

typedef struct hashtable{
	int (*keyHash)(void*);//ptr to key
	int (*keyCompare)(void*, void*);//ptr to key1, ptr to key2
	void (*deleteFunc)(void*, void*);//ptr to key, ptr to data
	hashentry** table;//the hash table
	int tableSize;
	int entryCount;
	//Deletion structures
	hashentry* first;//each hash entry points to the next one.
}hashtable;
extern int stringHash(void* key);
extern int stringCompare(void* key1, void* key2);
extern int ptrHash(void* key);
extern int ptrCompare(void* key1, void* key2);

extern int getHashIdx(hashtable* t, void* key);
extern void initHashtable(hashtable* t, int tableSize, int (*keyHash)(void*), int (*keyCompare)(void*, void*), void (*deleteFunc)(void*, void*));
extern void insertHash(hashtable* t, void* key, void* data);
extern int getData(hashtable* t, void* key, void** data);
extern void deleteAllEntries(hashtable* t);
extern void deleteEntry(hashtable* t, void* key);
extern void listKeys(hashtable* t);
extern void deleteHashtable(hashtable* t);

//Utility Functions
extern void printTableStatistics(hashtable* t);
extern void printAllHashes(hashtable* t);
#endif
