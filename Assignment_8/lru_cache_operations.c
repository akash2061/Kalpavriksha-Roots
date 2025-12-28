#include "lru_cache.h"

int hashFunction(int key)
{
    return ((key % cache->capacity) + cache->capacity) % cache->capacity;
}

QueueNode *createQueueNode(int key, const char *data)
{
    QueueNode *node = (QueueNode *)malloc(sizeof(QueueNode));
    if (node == NULL)
    {
        printf("Memory allocation failed\n");
        exit(1);
    }
    node->key = key;
    node->data = strdup(data);
    node->prev = NULL;
    node->next = NULL;
    return node;
}

void createCache(int capacity)
{
    if (capacity < 1 || capacity > MAX_CAPACITY)
    {
        printf("Error: Cache capacity must be between 1 and %d\n", MAX_CAPACITY);
        return;
    }

    cache = (LRUCache *)malloc(sizeof(LRUCache));
    if (cache == NULL)
    {
        printf("Memory allocation failed\n");
        exit(1);
    }

    cache->capacity = capacity;
    cache->size = 0;
    cache->front = NULL;
    cache->rear = NULL;

    cache->hashMap = (QueueNode **)calloc(capacity, sizeof(QueueNode *));
    if (cache->hashMap == NULL)
    {
        printf("Memory allocation failed\n");
        exit(1);
    }
}

int findInHashMap(int key)
{
    int index = hashFunction(key);
    int originalIndex = index;
    int i = 0;

    while (i < cache->capacity)
    {
        int probe = (originalIndex + i) % cache->capacity;

        if (cache->hashMap[probe] == NULL)
        {
            return -1;
        }

        if (cache->hashMap[probe] != DELETED && cache->hashMap[probe]->key == key)
        {
            return probe;
        }

        i++;
    }

    return -1;
}

void removeFromHashMap(int key)
{
    int index = findInHashMap(key);
    if (index != -1)
    {
        cache->hashMap[index] = DELETED;
    }
}

void insertToHashMap(QueueNode *node, int key)
{
    int index = hashFunction(key);
    int originalIndex = index;
    int i = 0;

    while (i < cache->capacity)
    {
        int probe = (originalIndex + i) % cache->capacity;

        if (cache->hashMap[probe] == NULL || cache->hashMap[probe] == DELETED)
        {
            cache->hashMap[probe] = node;
            return;
        }

        i++;
    }

    printf("ERROR: HashMap is full!\n");
}

void removeFromQueue(QueueNode *node)
{
    if (node->prev != NULL)
    {
        node->prev->next = node->next;
    }
    else
    {
        cache->front = node->next;
    }

    if (node->next != NULL)
    {
        node->next->prev = node->prev;
    }
    else
    {
        cache->rear = node->prev;
    }
}

void moveToFront(QueueNode *node)
{
    if (node->prev == NULL)
    {
        return;
    }

    removeFromQueue(node);

    node->next = cache->front;
    node->prev = NULL;

    if (cache->front != NULL)
    {
        cache->front->prev = node;
    }

    cache->front = node;

    if (cache->rear == NULL)
    {
        cache->rear = node;
    }
}

void addToFront(QueueNode *node)
{
    node->next = cache->front;
    node->prev = NULL;

    if (cache->front != NULL)
    {
        cache->front->prev = node;
    }

    cache->front = node;

    if (cache->rear == NULL)
    {
        cache->rear = node;
    }
}

QueueNode *insertToQueue(int key, const char *data)
{
    QueueNode *newNode = createQueueNode(key, data);

    if (cache->size >= cache->capacity)
    {
        QueueNode *lru = cache->rear;

        removeFromHashMap(lru->key);

        if (lru->prev != NULL)
        {
            cache->rear = lru->prev;
            cache->rear->next = NULL;
        }
        else
        {
            cache->front = cache->rear = NULL;
        }

        free(lru->data);
        free(lru);
        cache->size--;
    }

    addToFront(newNode);
    cache->size++;

    return newNode;
}

char *get(int key)
{
    int index = findInHashMap(key);

    if (index == -1)
    {
        return NULL;
    }

    QueueNode *node = cache->hashMap[index];

    moveToFront(node);

    return node->data;
}

void put(int key, const char *value)
{
    int index = findInHashMap(key);

    if (index != -1)
    {
        QueueNode *node = cache->hashMap[index];
        free(node->data);
        node->data = strdup(value);
        moveToFront(node);
    }
    else
    {
        QueueNode *newNode = insertToQueue(key, value);
        insertToHashMap(newNode, key);
    }
}

void freeCache()
{
    if (cache == NULL)
        return;

    QueueNode *current = cache->front;
    while (current != NULL)
    {
        QueueNode *temp = current;
        current = current->next;
        free(temp->data);
        free(temp);
    }

    free(cache->hashMap);
    free(cache);
    cache = NULL;
}