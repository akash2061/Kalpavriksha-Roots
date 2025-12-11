#include "fcfs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

HashNode *PCBHash[HASH_MAP_SIZE] = {0};
ReadyQueue readyQueue = {0};
WaitingQueue waitingQueue = {0};
TerminatedQueue terminatedQueue = {0};
KilledProcess *killedProcessListHead = NULL;

int getHashKey(int key)
{
    return key % HASH_MAP_SIZE;
}

void insertInPCB(int pid, char *pname, int burstTime, int ioTime, int ioDuration)
{
    ProcessDetails *newProcess = (ProcessDetails *)malloc(sizeof(ProcessDetails));
    if (!newProcess)
    {
        printf("\nMemory allocation failed");
    }
    newProcess->processID = pid;
    newProcess->processName = calloc(strlen(pname) + 1, sizeof(char));
    strcpy(newProcess->processName, pname);
    newProcess->burstTime = burstTime;
    newProcess->ioDuration = ioDuration;
    newProcess->ioStartTime = ioTime;
    newProcess->arrivalTime = 0;
    newProcess->completionTime = 0;
    newProcess->processState = READY;
    newProcess->turnAroundTime = 0;
    newProcess->waitingTime = 0;
    newProcess->runningTime = 0;
    newProcess->ioRemainingTime = 0;
    newProcess->killTime = -1;
    newProcess->next = NULL;
    newProcess->previous = NULL;

    if (readyQueue.rear == NULL)
    {
        readyQueue.rear = newProcess;
        readyQueue.front = newProcess;
    }
    else
    {
        readyQueue.rear->next = newProcess;
        newProcess->previous = readyQueue.rear;
        readyQueue.rear = newProcess;
    }

    HashNode *newHashNode = (HashNode *)malloc(sizeof(HashNode));
    if (!newHashNode)
    {
        printf("\nMemory allocation failed");
    }
    newHashNode->next = NULL;
    newHashNode->processData = newProcess;

    int hashIndex = getHashKey(pid);
    newHashNode->next = PCBHash[hashIndex];
    PCBHash[hashIndex] = newHashNode;
}

void updateKillTime(int pid, int time)
{
    int hashKey = getHashKey(pid);
    HashNode *currentHashNode = PCBHash[hashKey];
    int processExists = 0;
    while (currentHashNode != NULL)
    {
        if (currentHashNode->processData->processID == pid)
        {
            processExists = 1;
            break;
        }
        currentHashNode = currentHashNode->next;
    }
    if (processExists)
    {
        if (killedProcessListHead == NULL)
        {
            KilledProcess *newKilledProcess = (KilledProcess *)malloc(sizeof(KilledProcess));
            if (newKilledProcess == NULL)
            {
                printf("\nMemory allocation failed");
                return;
            }
            newKilledProcess->pid = pid;
            newKilledProcess->killTime = time;
            newKilledProcess->next = NULL;
            newKilledProcess->previous = NULL;
            killedProcessListHead = newKilledProcess;
            return;
        }

        KilledProcess *temp = killedProcessListHead;
        KilledProcess *beforeTemp = NULL;
        while (temp != NULL)
        {
            if (temp->pid == pid)
            {
                temp->killTime = time;
                break;
            }
            beforeTemp = temp;
            temp = temp->next;
        }
        if (temp == NULL)
        {
            KilledProcess *newKilledProcess = (KilledProcess *)malloc(sizeof(KilledProcess));
            if (newKilledProcess == NULL)
            {
                printf("\nMemory allocation failed");
                return;
            }
            newKilledProcess->pid = pid;
            newKilledProcess->killTime = time;
            beforeTemp->next = newKilledProcess;
            newKilledProcess->previous = beforeTemp;
            newKilledProcess->next = NULL;
        }
    }
    else
    {
        printf("\nError. Process with specified PID not found");
        return;
    }
}

void enqueueInQueue(ProcessDetails **qFront, ProcessDetails **qRear, ProcessDetails *process)
{
    if (*qRear == NULL)
    {
        *qRear = process;
        *qFront = process;
        process->previous = NULL;
    }
    else
    {
        (*qRear)->next = process;
        process->previous = *qRear;
        *qRear = process;
    }
    process->next = NULL;
}

void readLine(char *line)
{
    if (strlen(line) == 0)
    {
        return;
    }

    if (strncmp(line, "KILL", 4) == 0)
    {
        int pid, killTime;
        if (sscanf(line, "KILL %d %d", &pid, &killTime) == 2)
        {
            updateKillTime(pid, killTime);
        }
        else
        {
            printf("\nInvalid command");
        }
    }
    else
    {
        char name[50];
        int pid, burstTime, ioTime, ioDuration;
        if (sscanf(line, "%s %d %d %d %d", name, &pid, &burstTime, &ioTime, &ioDuration) == 5)
        {
            insertInPCB(pid, name, burstTime, ioTime, ioDuration);
        }
        else
        {
            printf("\nInvalid command");
        }
    }
}

void beginExecution()
{
    ProcessDetails *currentProcess = NULL;
    while (readyQueue.front != NULL || waitingQueue.front != NULL || processIsRunning)
    {
        checkKillProcess();
        if (currentProcess != NULL && currentProcess->processState == TERMINATED)
        {
            currentProcess = NULL;
        }
        if (!processIsRunning && readyQueue.front != NULL)
        {
            currentProcess = readyQueue.front;
            processIsRunning = 1;
            readyQueue.front = currentProcess->next;
            if (readyQueue.front == NULL)
            {
                readyQueue.rear = NULL;
            }
            else
            {
                readyQueue.front->previous = NULL;
            }
            currentProcess->processState = RUNNING;
            currentProcess->next = NULL;
            currentProcess->previous = NULL;
        }
        if (processIsRunning && currentProcess != NULL)
        {
            executeCurrentProcess(currentProcess);
            if (!processIsRunning)
            {
                currentProcess = NULL;
            }
        }
        increaseWaitingTime();
        systemClock++;
    }
}

void checkKillProcess()
{
    if (killedProcessListHead == NULL)
    {
        return;
    }

    KilledProcess *temp = killedProcessListHead;
    while (temp != NULL)
    {
        KilledProcess *nextProcess = temp->next;
        if (temp->killTime == systemClock)
        {
            int hashIndex = getHashKey(temp->pid);
            HashNode *currentNode = PCBHash[hashIndex];
            while (currentNode != NULL)
            {
                if (currentNode->processData->processID == temp->pid)
                {
                    ProcessDetails *currentProcess = currentNode->processData;
                    currentProcess->completionTime = systemClock + 1;

                    if (currentProcess->processState == RUNNING)
                    {
                        processIsRunning = 0;
                    }
                    else
                    {
                        if (readyQueue.front == currentProcess)
                        {
                            readyQueue.front = currentProcess->next;
                            if (readyQueue.front)
                            {
                                readyQueue.front->previous = NULL;
                            }
                            else
                            {
                                readyQueue.rear = NULL;
                            }
                        }
                        else if (readyQueue.rear == currentProcess)
                        {
                            readyQueue.rear = currentProcess->previous;
                            if (readyQueue.rear)
                            {
                                readyQueue.rear->next = NULL;
                            }
                            else
                            {
                                readyQueue.front = NULL;
                            }
                        }
                        else if (waitingQueue.front == currentProcess)
                        {
                            waitingQueue.front = currentProcess->next;
                            if (waitingQueue.front)
                            {
                                waitingQueue.front->previous = NULL;
                            }
                            else
                            {
                                waitingQueue.rear = NULL;
                            }
                        }
                        else if (waitingQueue.rear == currentProcess)
                        {
                            waitingQueue.rear = currentProcess->previous;
                            if (waitingQueue.rear)
                            {
                                waitingQueue.rear->next = NULL;
                            }
                            else
                            {
                                waitingQueue.front = NULL;
                            }
                        }
                        else
                        {
                            currentProcess->previous->next = currentProcess->next;
                            currentProcess->next->previous = currentProcess->previous;
                        }
                    }
                    currentProcess->next = currentProcess->previous = NULL;
                    currentProcess->processState = TERMINATED;
                    currentProcess->waitingTime = currentProcess->turnAroundTime = -1;
                    currentProcess->killTime = temp->killTime;

                    enqueueInQueue(&terminatedQueue.front, &terminatedQueue.rear, currentProcess);
                    break;
                }
                currentNode = currentNode->next;
            }
            if (temp->previous)
            {
                temp->previous->next = temp->next;
            }
            else
            {
                killedProcessListHead = temp->next;
            }
            if (temp->next)
            {
                temp->next->previous = temp->previous;
            }

            free(temp);
        }
        temp = nextProcess;
    }
}

void executeCurrentProcess(ProcessDetails *currentProcess)
{
    currentProcess->runningTime++;
    if (currentProcess->runningTime == currentProcess->burstTime)
    {
        processIsRunning = 0;
        currentProcess->processState = TERMINATED;
        currentProcess->completionTime = systemClock + 1;
        currentProcess->turnAroundTime = currentProcess->completionTime -
                                         currentProcess->arrivalTime;
        enqueueInQueue(&terminatedQueue.front, &terminatedQueue.rear, currentProcess);
        return;
    }

    if (currentProcess->runningTime == currentProcess->ioStartTime)
    {
        processIsRunning = 0;
        currentProcess->waitingTime--;
        currentProcess->processState = WAITING;
        currentProcess->ioRemainingTime = currentProcess->ioDuration + 1;
        enqueueInQueue(&waitingQueue.front, &waitingQueue.rear, currentProcess);
    }
}

void increaseWaitingTime()
{
    if (readyQueue.front != NULL)
    {
        ProcessDetails *temp = readyQueue.front;
        while (temp != NULL)
        {
            temp->waitingTime++;
            temp = temp->next;
        }
    }
    if (waitingQueue.front != NULL)
    {
        ProcessDetails *temp = waitingQueue.front;
        while (temp != NULL)
        {
            temp->waitingTime++;
            temp->ioRemainingTime--;

            ProcessDetails *nextProcess = temp->next;
            if (temp->ioRemainingTime == 0)
            {
                temp->processState = READY;
                temp->ioRemainingTime = temp->ioDuration;
                if (temp->previous)
                {
                    temp->previous->next = temp->next;
                }
                else
                {
                    waitingQueue.front = temp->next;
                }

                if (temp->next)
                {
                    temp->next->previous = temp->previous;
                }
                else
                {
                    waitingQueue.rear = temp->previous;
                }
                temp->next = temp->previous = NULL;
                enqueueInQueue(&readyQueue.front, &readyQueue.rear, temp);
            }
            temp = nextProcess;
        }
    }
}

void printOutput()
{
    if (terminatedQueue.front == NULL)
        return;

    int count = 0;
    ProcessDetails *temp = terminatedQueue.front;
    int anyKilled = 0;
    while (temp != NULL)
    {
        count++;
        if (temp->killTime != -1)
            anyKilled = 1;
        temp = temp->next;
    }

    ProcessDetails **arr = malloc(count * sizeof(ProcessDetails *));
    temp = terminatedQueue.front;
    int i = 0;
    while (temp != NULL)
    {
        arr[i++] = temp;
        temp = temp->next;
    }

    for (int first = 0; first < count - 1; first++)
    {
        for (int second = first + 1; second < count; second++)
        {
            if (arr[first]->processID > arr[second]->processID)
            {
                ProcessDetails *swap = arr[first];
                arr[first] = arr[second];
                arr[second] = swap;
            }
        }
    }

    if (anyKilled)
        printf("\n| %-6s | %-15s | %-6s | %-6s | %-12s | %-15s | %-15s |",
               "PID", "Name", "CPU", "IO", "Status", "Turnaround", "Waiting");
    else
        printf("\n| %-6s | %-15s | %-6s | %-6s | %-15s | %-15s |",
               "PID", "Name", "CPU", "IO", "Turnaround", "Waiting");

    for (int inx = 0; inx < count; inx++)
    {
        ProcessDetails *currentProcess = arr[inx];

        if (anyKilled)
        {
            if (currentProcess->killTime != -1)
            {
                printf("\n| %-6d | %-15s | %-6d | %-6d | %-9s %-2d | %-15s | %-15s |",
                       currentProcess->processID, currentProcess->processName, currentProcess->burstTime, currentProcess->ioDuration,
                       "KILLED at", currentProcess->killTime, "-", "-");
            }
            else
            {
                printf("\n| %-6d | %-15s | %-6d | %-6d | %-12s | %-15d | %-15d |",
                       currentProcess->processID, currentProcess->processName, currentProcess->burstTime, currentProcess->ioDuration,
                       "OK", currentProcess->turnAroundTime, currentProcess->waitingTime);
            }
        }
        else
        {
            printf("\n| %-6d | %-15s | %-6d | %-6d | %-15d | %-15d |",
                   currentProcess->processID, currentProcess->processName, currentProcess->burstTime,
                   currentProcess->ioDuration, currentProcess->turnAroundTime, currentProcess->waitingTime);
        }
    }
    free(arr);
}