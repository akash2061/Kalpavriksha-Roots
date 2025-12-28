#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "fcfs.h"

int systemClock = 0;
int processIsRunning = 0;

int main()
{
    char inputBuffer[INPUT_SIZE];
    while (1)
    {
        if (!fgets(inputBuffer, INPUT_SIZE, stdin))
            break;
        inputBuffer[strcspn(inputBuffer, "\n")] = '\0';
        if (strcmp(inputBuffer, "exit") == 0)
        {
            break;
        }
        readLine(inputBuffer);
    }
    beginExecution();
    printOutput();
    return 0;
}
