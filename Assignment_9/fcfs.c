#include "fcfs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

HashNode *PCBHash[HASH_MAP_SIZE] = {0};
Queue readyQueue = {0};
Queue waitingQueue = {0};
Queue terminatedQueue = {0};
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
    newProcess->process_id = pid;
    newProcess->process_name = calloc(strlen(pname) + 1, sizeof(char));
    strcpy(newProcess->process_name, pname);
    newProcess->burst_time = burstTime;
    newProcess->io_duration = ioDuration;
    newProcess->io_start_time = ioTime;
    newProcess->arrival_time = 0;
    newProcess->completion_time = 0;
    newProcess->process_state = READY;
    newProcess->turn_around_time = 0;
    newProcess->waiting_time = 0;
    newProcess->running_time = 0;
    newProcess->io_remaining_time = 0;
    newProcess->kill_time = -1;
    newProcess->next = NULL;

    if (readyQueue.rear == NULL)
    {
        readyQueue.rear = newProcess;
        readyQueue.front = newProcess;
    }
    else
    {
        readyQueue.rear->next = newProcess;
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
        if (currentHashNode->processData->process_id == pid)
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
    }
    else
    {
        (*qRear)->next = process;
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
        if (currentProcess != NULL && currentProcess->process_state == TERMINATED)
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
            currentProcess->process_state = RUNNING;
            currentProcess->next = NULL;
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
    KilledProcess *prevKilled = NULL;
    while (temp != NULL)
    {
        KilledProcess *nextProcess = temp->next;
        if (temp->killTime == systemClock)
        {
            int hashIndex = getHashKey(temp->pid);
            HashNode *currentNode = PCBHash[hashIndex];
            while (currentNode != NULL)
            {
                if (currentNode->processData->process_id == temp->pid)
                {
                    ProcessDetails *currentProcess = currentNode->processData;
                    currentProcess->completion_time = systemClock + 1;

                    if (currentProcess->process_state == RUNNING)
                    {
                        processIsRunning = 0;
                    }
                    else
                    {
                        if (readyQueue.front == currentProcess)
                        {
                            readyQueue.front = currentProcess->next;
                            if (readyQueue.front == NULL)
                            {
                                readyQueue.rear = NULL;
                            }
                        }
                        else if (waitingQueue.front == currentProcess)
                        {
                            waitingQueue.front = currentProcess->next;
                            if (waitingQueue.front == NULL)
                            {
                                waitingQueue.rear = NULL;
                            }
                        }
                        else
                        {
                            ProcessDetails *prev = NULL;
                            ProcessDetails *curr = readyQueue.front;
                            if (curr == NULL)
                                curr = waitingQueue.front;
                            while (curr != NULL && curr != currentProcess)
                            {
                                prev = curr;
                                curr = curr->next;
                            }
                            if (curr != NULL && prev != NULL)
                            {
                                prev->next = curr->next;
                                if (curr == readyQueue.rear)
                                    readyQueue.rear = prev;
                                if (curr == waitingQueue.rear)
                                    waitingQueue.rear = prev;
                            }
                        }
                    }
                    currentProcess->next = NULL;
                    currentProcess->process_state = TERMINATED;
                    currentProcess->waiting_time = currentProcess->turn_around_time = -1;
                    currentProcess->kill_time = temp->killTime;

                    enqueueInQueue(&terminatedQueue.front, &terminatedQueue.rear, currentProcess);
                    break;
                }
                currentNode = currentNode->next;
            }
            if (prevKilled)
            {
                prevKilled->next = temp->next;
            }
            else
            {
                killedProcessListHead = temp->next;
            }
            free(temp);
        }
        else
        {
            prevKilled = temp;
        }
        temp = nextProcess;
    }
}

void executeCurrentProcess(ProcessDetails *currentProcess)
{
    currentProcess->running_time++;
    if (currentProcess->running_time == currentProcess->burst_time)
    {
        processIsRunning = 0;
        currentProcess->process_state = TERMINATED;
        currentProcess->completion_time = systemClock + 1;
        currentProcess->turn_around_time = currentProcess->completion_time -
                                           currentProcess->arrival_time;
        enqueueInQueue(&terminatedQueue.front, &terminatedQueue.rear, currentProcess);
        return;
    }

    if (currentProcess->running_time == currentProcess->io_start_time)
    {
        processIsRunning = 0;
        currentProcess->waiting_time--;
        currentProcess->process_state = WAITING;
        currentProcess->io_remaining_time = currentProcess->io_duration + 1;
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
            temp->waiting_time++;
            temp = temp->next;
        }
    }
    if (waitingQueue.front != NULL)
    {
        ProcessDetails *temp = waitingQueue.front;
        ProcessDetails *prev = NULL;
        while (temp != NULL)
        {
            temp->waiting_time++;
            temp->io_remaining_time--;

            ProcessDetails *nextProcess = temp->next;
            if (temp->io_remaining_time == 0)
            {
                temp->process_state = READY;
                temp->io_remaining_time = temp->io_duration;
                if (prev)
                {
                    prev->next = temp->next;
                }
                else
                {
                    waitingQueue.front = temp->next;
                }
                if (temp->next == NULL)
                {
                    waitingQueue.rear = prev;
                }
                temp->next = NULL;
                enqueueInQueue(&readyQueue.front, &readyQueue.rear, temp);
            }
            else
            {
                prev = temp;
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
        if (temp->kill_time != -1)
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
            if (arr[first]->process_id > arr[second]->process_id)
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
            if (currentProcess->kill_time != -1)
            {
                printf("\n| %-6d | %-15s | %-6d | %-6d | %-9s %-2d | %-15s | %-15s |",
                       currentProcess->process_id, currentProcess->process_name, currentProcess->burst_time, currentProcess->io_duration,
                       "KILLED at", currentProcess->kill_time, "-", "-");
            }
            else
            {
                printf("\n| %-6d | %-15s | %-6d | %-6d | %-12s | %-15d | %-15d |",
                       currentProcess->process_id, currentProcess->process_name, currentProcess->burst_time, currentProcess->io_duration,
                       "OK", currentProcess->turn_around_time, currentProcess->waiting_time);
            }
        }
        else
        {
            printf("\n| %-6d | %-15s | %-6d | %-6d | %-15d | %-15d |",
                   currentProcess->process_id, currentProcess->process_name, currentProcess->burst_time,
                   currentProcess->io_duration, currentProcess->turn_around_time, currentProcess->waiting_time);
        }
    }
    free(arr);
}