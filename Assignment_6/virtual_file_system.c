#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NAME_LENGTH 50
#define MAX_BLOCK_SIZE 512
#define MAX_BLOCK_COUNT 1024
#define MAX_COMMAND_LENGTH 2048
#define MAX_BLOCKS_PER_FILE 100
#define TRUE 1

typedef struct FreeBlock
{
    int block_index;
    struct FreeBlock *next;
    struct FreeBlock *previous;
} FreeBlock;

typedef struct FileNode
{
    char name[MAX_NAME_LENGTH];
    int is_directory;
    int size;
    int block_count;
    struct FileNode *child;
    struct FileNode *previous;
    struct FileNode *next;
    struct FileNode *parent;
    int block_pointers[MAX_BLOCKS_PER_FILE];
} FileNode;

unsigned char virtual_disk[MAX_BLOCK_COUNT][MAX_BLOCK_SIZE];
FreeBlock *freeListHead = NULL;
FreeBlock *freeListTail = NULL;
int used_blocks = 0;

void createRootDirectory(FileNode **);
void traverseDirectory(FileNode *);
void cmdMkdir(const char *, FileNode *);
void cmdRmdir(FileNode **, const char *);
void cmdLs(FileNode *);
void cmdCd(FileNode **, const char *);
void cmdPwd(FileNode *);
void cmdDf(FileNode *);
void cmdCreate(const char *, FileNode *);
void cmdRead(const char *, FileNode *);
void cmdWrite(const char *, const char *, FileNode *);
void cmdDelete(const char *, FileNode *);
void cmdExit(FileNode *);

FreeBlock *allocateBlock();
void initializeFreeList();
void freeAllocatedBlocks(int);
void freeAllNodes(FileNode *);

void runCmd(const char *, const char *, char *, FileNode **, FileNode *);
char *extractData(const char *, const char *, char *);
FileNode *isFilePresent(FileNode *, const char *);

int main()
{
    char command[MAX_COMMAND_LENGTH];
    FileNode *root = NULL;
    createRootDirectory(&root);
    if (!root)
    {
        printf("Error: Memory allocation failed\n");
        return -1;
    }
    initializeFreeList();
    FileNode *current_directory = root;
    printf("Compact VFS - ready. Type 'exit' to quit.\n");
    while (TRUE)
    {
        printf("%s> ", current_directory->name);
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = '\0';

        if (strlen(command) == 0)
        {
            continue;
        }

        char command_copy[MAX_COMMAND_LENGTH];
        strcpy(command_copy, command);

        char *cmd = strtok(command_copy, " ");
        char *arg = strtok(NULL, " ");
        char *data_string = NULL;

        if (cmd == NULL)
        {
            continue;
        }

        if (strcmp(cmd, "write") == 0)
        {
            data_string = extractData(cmd, arg, command);
        }

        runCmd(cmd, arg, data_string, &current_directory, root);
    }
    return 0;
}

void runCmd(const char *cmd, const char *arg, char *data_string, FileNode **current_directory, FileNode *root)
{
    if (strcmp(cmd, "mkdir") == 0)
    {
        if (arg == NULL)
        {
            printf("Error: mkdir requires a directory name.\n");
        }
        else
        {
            cmdMkdir(arg, *current_directory);
        }
    }
    else if (strcmp(cmd, "create") == 0)
    {
        if (arg == NULL)
        {
            printf("Error: create requires a file name.\n");
        }
        else
        {
            cmdCreate(arg, *current_directory);
        }
    }
    else if (strcmp(cmd, "write") == 0)
    {
        if (arg == NULL)
        {
            printf("Error: write requires filename and data.\n");
        }
        else if (data_string == NULL)
        {
            printf("Error: Invalid write syntax. Use: write <filename> \"data\"\n");
        }
        else
        {
            cmdWrite(arg, data_string, *current_directory);
            free(data_string);
            data_string = NULL;
        }
    }
    else if (strcmp(cmd, "read") == 0)
    {
        if (arg == NULL)
        {
            printf("Error: read requires a file name.\n");
        }
        else
        {
            cmdRead(arg, *current_directory);
        }
    }
    else if (strcmp(cmd, "delete") == 0)
    {
        if (arg == NULL)
        {
            printf("Error: delete requires a file name.\n");
        }
        else
        {
            cmdDelete(arg, *current_directory);
        }
    }
    else if (strcmp(cmd, "ls") == 0)
    {
        cmdLs(*current_directory);
    }
    else if (strcmp(cmd, "cd") == 0)
    {
        if (arg == NULL)
        {
            printf("Error: cd requires a directory name.\n");
        }
        else
        {
            cmdCd(current_directory, arg);
        }
    }
    else if (strcmp(cmd, "pwd") == 0)
    {
        cmdPwd(*current_directory);
        printf("\n");
    }
    else if (strcmp(cmd, "df") == 0)
    {
        cmdDf(root);
    }
    else if (strcmp(cmd, "rmdir") == 0)
    {
        if (arg == NULL)
        {
            printf("Error: rmdir requires a directory name.\n");
        }
        else
        {
            cmdRmdir(current_directory, arg);
        }
    }
    else if (strcmp(cmd, "exit") == 0)
    {
        cmdExit(root);
        exit(0);
    }
    else
    {
        printf("Unknown command.\n");
    }
}

char *extractData(const char *cmd, const char *arg, char *command)
{
    char *data_string = NULL;

    if (arg != NULL)
    {
        char *quote_start = strchr(command, '"');
        if (quote_start != NULL)
        {
            char *quote_end = strchr(quote_start + 1, '"');
            if (quote_end != NULL)
            {
                int data_length = quote_end - quote_start - 1;
                data_string = (char *)malloc(data_length + 1);
                if (data_string != NULL)
                {
                    strncpy(data_string, quote_start + 1, data_length);
                    data_string[data_length] = '\0';
                }
            }
        }
    }
    return data_string;
}

FileNode *isFilePresent(FileNode *current_directory, const char *filename)
{
    if (current_directory->child == NULL)
    {
        return NULL;
    }

    FileNode *temp = current_directory->child;
    FileNode *file = NULL;
    do
    {
        if (!temp->is_directory && strcmp(temp->name, filename) == 0)
        {
            file = temp;
            break;
        }
        temp = temp->next;
    } while (temp != current_directory->child);

    return file;
}

void createRootDirectory(FileNode **root)
{
    *root = (FileNode *)malloc(sizeof(FileNode));
    if (!(*root))
    {
        printf("Error: Memory allocation failed\n");
        exit(-1);
    }
    strcpy((*root)->name, "/");
    (*root)->is_directory = 1;
    (*root)->size = 0;
    (*root)->block_count = 0;
    (*root)->child = NULL;
    (*root)->previous = NULL;
    (*root)->next = NULL;
    (*root)->parent = NULL;
}

void cmdPwd(FileNode *current_directory)
{
    if (current_directory->parent != NULL)
    {
        cmdPwd(current_directory->parent);
        printf("%s/", current_directory->name);
    }
    else
    {
        printf("/");
    }
}

void cmdMkdir(const char *name, FileNode *parent)
{
    if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
    {
        printf("Error: cannot create directory '%s'\n", name);
        return;
    }

    FileNode *new_dir = (FileNode *)malloc(sizeof(FileNode));
    if (!new_dir)
    {
        printf("Error: Memory allocation failed\n");
        return;
    }
    if (parent->child != NULL)
    {
        FileNode *temp = parent->child;
        do
        {
            if (strcmp(temp->name, name) == 0)
            {
                printf("Error: Directory '%s' already exists\n", name);
                free(new_dir);
                return;
            }
            temp = temp->next;
        } while (temp != parent->child);
    }
    strcpy(new_dir->name, name);
    new_dir->is_directory = 1;
    new_dir->size = 0;
    new_dir->block_count = 0;
    new_dir->child = NULL;
    new_dir->parent = parent;

    if (parent->child == NULL)
    {
        new_dir->previous = new_dir;
        new_dir->next = new_dir;
        parent->child = new_dir;
    }
    else
    {
        FileNode *last_child = parent->child->previous;
        new_dir->previous = last_child;
        new_dir->next = parent->child;
        last_child->next = new_dir;
        parent->child->previous = new_dir;
    }
    printf("Directory '%s' created successfully\n", name);
}

void cmdCreate(const char *name, FileNode *parent)
{
    if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
    {
        printf("Error: cannot create file '%s'\n", name);
        return;
    }

    FileNode *new_file = (FileNode *)malloc(sizeof(FileNode));
    if (!new_file)
    {
        printf("Error: Memory allocation failed\n");
        return;
    }
    if (parent->child != NULL)
    {
        FileNode *temp = parent->child;
        do
        {
            if (strcmp(temp->name, name) == 0)
            {
                printf("Error: File '%s' already exists\n", name);
                free(new_file);
                return;
            }
            temp = temp->next;
        } while (temp != parent->child);
    }
    strcpy(new_file->name, name);
    new_file->is_directory = 0;
    new_file->size = 0;
    new_file->block_count = 0;
    new_file->child = NULL;
    new_file->parent = parent;

    if (parent->child == NULL)
    {
        new_file->previous = new_file;
        new_file->next = new_file;
        parent->child = new_file;
    }
    else
    {
        FileNode *last_child = parent->child->previous;
        new_file->previous = last_child;
        new_file->next = parent->child;
        last_child->next = new_file;
        parent->child->previous = new_file;
    }
    printf("File '%s' created successfully\n", name);
}

void traverseDirectory(FileNode *directory)
{
    if (directory == NULL)
        return;

    if (directory->child == NULL)
    {
        printf("(empty)\n");
        return;
    }

    FileNode *child = directory->child;
    do
    {
        if (child->is_directory)
        {
            printf("%s/\n", child->name);
        }
        else
        {
            printf("%s\n", child->name);
        }
        child = child->next;
    } while (child != directory->child);
}

void cmdLs(FileNode *current_directory)
{
    traverseDirectory(current_directory);
}

void cmdCd(FileNode **current_directory, const char *dirname)
{
    if (strcmp(dirname, "..") == 0)
    {
        if ((*current_directory)->parent != NULL)
        {
            *current_directory = (*current_directory)->parent;
            printf("Moved to %s\n", (*current_directory)->name);
        }
        else
        {
            printf("Already at root directory\n");
        }
        return;
    }

    if (strcmp(dirname, ".") == 0)
    {
        printf("Already in directory: %s\n", (*current_directory)->name);
        return;
    }

    if ((*current_directory)->child == NULL)
    {
        printf("Error: Directory '%s' not found\n", dirname);
        return;
    }

    FileNode *temp = (*current_directory)->child;
    do
    {
        if (temp->is_directory && strcmp(temp->name, dirname) == 0)
        {
            *current_directory = temp;
            printf("Moved to ");
            cmdPwd(*current_directory);
            printf("\n");
            return;
        }
        temp = temp->next;
    } while (temp != (*current_directory)->child);

    printf("Error: Directory '%s' not found\n", dirname);
}

void cmdDf(FileNode *root)
{
    int freeBlocks = MAX_BLOCK_COUNT - used_blocks;
    float usage = (used_blocks * 100.0) / MAX_BLOCK_COUNT;

    printf("Total Blocks: %d\n", MAX_BLOCK_COUNT);
    printf("Used Blocks: %d\n", used_blocks);
    printf("Free Blocks: %d\n", freeBlocks);
    printf("Disk Usage: %.2f%%\n", usage);
}

void cmdRmdir(FileNode **current_directory, const char *dirname)
{
    if ((*current_directory)->child == NULL)
    {
        printf("Error: Directory '%s' not found\n", dirname);
        return;
    }
    FileNode *start = (*current_directory)->child;
    FileNode *temp = start;
    do
    {
        if (temp->is_directory && strcmp(temp->name, dirname) == 0)
        {
            if (temp->child != NULL)
            {
                printf("Error: Directory '%s' is not empty\n", dirname);
                return;
            }
            if (temp->next == temp && temp->previous == temp)
            {
                (*current_directory)->child = NULL;
            }
            else
            {
                temp->previous->next = temp->next;
                temp->next->previous = temp->previous;
                if ((*current_directory)->child == temp)
                {
                    (*current_directory)->child = temp->next;
                }
            }
            free(temp);
            printf("Directory '%s' removed successfully\n", dirname);
            return;
        }
        temp = temp->next;
    } while (temp != start);
    printf("Error: Directory '%s' not found\n", dirname);
}

void cmdWrite(const char *filename, const char *data, FileNode *current_directory)
{
    FileNode *file = isFilePresent(current_directory, filename);

    if (file == NULL)
    {
        printf("Error: File '%s' not found.\n", filename);
        return;
    }

    if (file->is_directory)
    {
        printf("Error: '%s' is a directory.\n", filename);
        return;
    }

    for (int i = 0; i < file->block_count; i++)
    {
        freeAllocatedBlocks(file->block_pointers[i]);
        file->block_pointers[i] = 0;
    }
    file->block_count = 0;
    file->size = 0;

    int dataLen = strlen(data);
    int blocksNeeded = (dataLen + MAX_BLOCK_SIZE - 1) / MAX_BLOCK_SIZE;

    if (blocksNeeded > MAX_BLOCKS_PER_FILE)
    {
        printf("Error: File size exceeds maximum blocks per file.\n");
        return;
    }

    if (blocksNeeded > MAX_BLOCK_COUNT - used_blocks)
    {
        printf("Error: Not enough disk space.\n");
        return;
    }

    int currentBlock = 0;
    for (int i = 0; i < dataLen; i++)
    {
        if (i % MAX_BLOCK_SIZE == 0)
        {
            FreeBlock *block = allocateBlock();
            if (block == NULL)
            {
                printf("Error: Disk full.\n");
                return;
            }
            file->block_pointers[currentBlock] = block->block_index;
            file->block_count++;
            free(block);
            currentBlock++;
        }

        int current_block_index = file->block_pointers[i / MAX_BLOCK_SIZE];
        int positionInBlock = i % MAX_BLOCK_SIZE;
        virtual_disk[current_block_index][positionInBlock] = data[i];
    }

    file->size = dataLen;
    printf("Data written successfully (size=%d bytes).\n", dataLen);
}

void cmdRead(const char *filename, FileNode *current_directory)
{
    FileNode *file = isFilePresent(current_directory, filename);

    if (file == NULL)
    {
        printf("Error: File '%s' not found.\n", filename);
        return;
    }

    if (file->is_directory)
    {
        printf("Error: '%s' is a directory.\n", filename);
        return;
    }

    if (file->size == 0)
    {
        printf("(empty file)\n");
        return;
    }

    int bytesRead = 0;
    for (int i = 0; i < file->block_count; i++)
    {
        int blockIndex = file->block_pointers[i];

        for (int j = 0; j < MAX_BLOCK_SIZE && bytesRead < file->size; j++)
        {
            printf("%c", virtual_disk[blockIndex][j]);
            bytesRead++;
        }
    }
    printf("\n");
}

void cmdDelete(const char *filename, FileNode *current_directory)
{
    FileNode *file = isFilePresent(current_directory, filename);

    if (file == NULL)
    {
        printf("Error: File '%s' not found.\n", filename);
        return;
    }

    if (file->is_directory)
    {
        printf("Error: '%s' is a directory. Use rmdir.\n", filename);
        return;
    }

    for (int i = 0; i < file->block_count; i++)
    {
        freeAllocatedBlocks(file->block_pointers[i]);
    }

    if (file->next == file && file->previous == file)
    {
        current_directory->child = NULL;
    }
    else
    {
        file->previous->next = file->next;
        file->next->previous = file->previous;
        if (current_directory->child == file)
        {
            current_directory->child = file->next;
        }
    }
    free(file);

    printf("File deleted successfully.\n");
}

void initializeFreeList()
{
    for (int i = 0; i < MAX_BLOCK_COUNT; i++)
    {
        FreeBlock *block = (FreeBlock *)malloc(sizeof(FreeBlock));
        block->block_index = i;
        block->next = NULL;
        block->previous = freeListTail;

        if (freeListHead == NULL)
        {
            freeListHead = block;
        }
        if (freeListTail != NULL)
        {
            freeListTail->next = block;
        }
        freeListTail = block;
    }
}

FreeBlock *allocateBlock()
{
    if (freeListHead == NULL)
    {
        return NULL;
    }

    FreeBlock *block = freeListHead;
    freeListHead = block->next;

    if (freeListHead != NULL)
    {
        freeListHead->previous = NULL;
    }
    else
    {
        freeListTail = NULL;
    }

    used_blocks++;
    return block;
}

void freeAllocatedBlocks(int to_free_block)
{
    FreeBlock *block = (FreeBlock *)malloc(sizeof(FreeBlock));
    block->block_index = to_free_block;
    block->next = NULL;
    block->previous = freeListTail;

    if (freeListTail != NULL)
    {
        freeListTail->next = block;
    }
    if (freeListHead == NULL)
    {
        freeListHead = block;
    }
    freeListTail = block;

    used_blocks--;
}

void freeAllNodes(FileNode *node)
{
    if (node == NULL)
        return;

    if (node->is_directory && node->child != NULL)
    {
        FileNode *current = node->child;
        FileNode *start = current;
        do
        {
            FileNode *next = current->next;
            freeAllNodes(current);
            if (next == start)
                break;
            current = next;
        } while (TRUE);
    }

    if (!node->is_directory)
    {
        for (int i = 0; i < node->block_count; i++)
        {
            freeAllocatedBlocks(node->block_pointers[i]);
        }
    }

    free(node);
}

void cmdExit(FileNode *root)
{
    if (root != NULL)
    {
        freeAllNodes(root);
    }

    while (freeListHead != NULL)
    {
        FreeBlock *temp = freeListHead;
        freeListHead = freeListHead->next;
        free(temp);
    }

    printf("Memory released. Exiting program...\n");
}
