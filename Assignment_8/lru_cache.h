#ifndef LRU_CACHE_H
#define LRU_CACHE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STRING_LENGTH 256
#define MAX_CAPACITY 1000
#define DELETED (QueueNode *)0xFFFFFFFFFFFFFFFF
#define TRUE 1

typedef struct QueueNode
{
    int key;
    char *data;
    struct QueueNode *prev;
    struct QueueNode *next;
} QueueNode;

typedef struct
{
    int capacity;
    int size;
    QueueNode *front;
    QueueNode *rear;
    QueueNode **hashMap;
} LRUCache;

extern LRUCache *cache;

int hashFunction(int);
QueueNode *createQueueNode(int, const char *);
void createCache(int);
int findInHashMap(int);
void insertToHashMap(QueueNode *, int);
void removeFromQueue(QueueNode *);
void moveToFront(QueueNode *);
void addToFront(QueueNode *);
QueueNode *insertToQueue(int, const char *);
char *get(int);
void put(int, const char *);
void freeCache();

#endif