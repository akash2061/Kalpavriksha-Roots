#include "lru_cache.h"

LRUCache *cache = NULL;

int main()
{
    char command[MAX_STRING_LENGTH];
    int key, capacity;
    char value[MAX_STRING_LENGTH];

    while (TRUE)
    {
        scanf("%s", command);

        if (strcmp(command, "createCache") == 0)
        {
            scanf("%d", &capacity);
            if (capacity < 1 || capacity > MAX_CAPACITY)
            {
                printf("Capacity must be between 1 and %d\n", MAX_CAPACITY);
                continue;
            }
            createCache(capacity);
        }
        else if (strcmp(command, "put") == 0)
        {
            scanf("%d %s", &key, value);
            put(key, value);
        }
        else if (strcmp(command, "get") == 0)
        {
            scanf("%d", &key);
            char *result = get(key);
            printf("%s\n", result != NULL ? result : "NULL");
        }
        else if (strcmp(command, "exit") == 0)
        {
            break;
        }
    }

    freeCache();
    return 0;
}