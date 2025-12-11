#ifndef FCFS_H
#define FCFS_H

#define HASH_MAP_SIZE 100
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
    int processID;
    char *processName;
    int burstTime;
    int ioStartTime;
    int ioDuration;
    State processState;
    int arrivalTime;
    int completionTime;
    int waitingTime;
    int turnAroundTime;
    int runningTime;
    int ioRemainingTime;
    int killTime;
    struct ProcessDetails *next;
    struct ProcessDetails *previous;
} ProcessDetails;

typedef struct
{
    ProcessDetails *front;
    ProcessDetails *rear;
} ReadyQueue;

typedef struct
{
    ProcessDetails *front;
    ProcessDetails *rear;
} WaitingQueue;

typedef struct
{
    ProcessDetails *front;
    ProcessDetails *rear;
} TerminatedQueue;

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
extern ReadyQueue readyQueue;
extern WaitingQueue waitingQueue;
extern TerminatedQueue terminatedQueue;
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