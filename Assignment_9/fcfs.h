#ifndef FCFS_H
#define FCFS_H

#define HASH_MAP_SIZE 101
#define INPUT_SIZE 100

typedef enum
{
    READY,
    RUNNING,
    WAITING,
    TERMINATED
} State;

typedef struct ProcessDetails
{
    int process_id;
    char *process_name;
    int burst_time;
    int io_start_time;
    int io_duration;
    State process_state;
    int arrival_time;
    int completion_time;
    int waiting_time;
    int turn_around_time;
    int running_time;
    int io_remaining_time;
    int kill_time;
    struct ProcessDetails *next;
} ProcessDetails;

typedef struct
{
    ProcessDetails *front;
    ProcessDetails *rear;
} Queue;

typedef struct HashNode
{
    ProcessDetails *processData;
    struct HashNode *next;
} HashNode;

typedef struct KilledProcess
{
    int pid;
    int killTime;
    struct KilledProcess *next;
    struct KilledProcess *previous;
} KilledProcess;

extern HashNode *PCBHash[HASH_MAP_SIZE];
extern Queue readyQueue;
extern Queue waitingQueue;
extern Queue terminatedQueue;
extern KilledProcess *killedProcessListHead;
extern int systemClock;
extern int processIsRunning;

int getHashKey(int);
void insertInPCB(int, char *, int, int, int);
void updateKillTime(int, int);
void enqueueInQueue(ProcessDetails **, ProcessDetails **, ProcessDetails *);
void readLine(char *);
void beginExecution();
void checkKillProcess();
void executeCurrentProcess(ProcessDetails *);
void increaseWaitingTime();
void printOutput();

#endif