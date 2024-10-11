

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <assert.h>
#include <time.h>
#include "MallocCheck.h"
#include "SharedStruct.h"

#define SEM_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
#define TABLESIZE 25

// Prototypes
void ReadSemaphores(void);
void PrintSemaphoreValues(void);
void InitSemaphores(void);
void PrintArrays(SharedStruct *);
void PrintNumbers(SharedStruct *);
void PrintCounts(SharedStruct *);

// Semaphores
sem_t *mutex;
sem_t *wrt;
sem_t *queue;

int mutex_Value;
int wrt_Value;
int queue_Value;


int main(void)
{
    int SegmentID;
    SharedStruct *ss;

    printf("This is the Print Stats program.\n");

    // Initialize random number generator
    srand(time(NULL));

    printf("Please enter the shared memory segment's ID: ");
    scanf("%d", &SegmentID);

    // Attach segment
    ss = AttachSegment(SegmentID);

    // Initialize semaphores
    InitSemaphores();

    // Print stats
    for (int i = 0; i < 250; i++)
    {   
        PrintSemaphoreValues();
        PrintCounts(ss);
        PrintArrays(ss);
        PrintNumbers(ss);

        sleep(1);
        system("clear");
    }

    // Detach everything
    DetachSegment(ss);
    sem_close(mutex);
    sem_close(wrt);
    sem_close(queue);
    
    return 0;
}

void ReadSemaphores(void)
{
    sem_getvalue(mutex, &mutex_Value);
    sem_getvalue(wrt, &wrt_Value);
    sem_getvalue(queue, &queue_Value);
}

void PrintSemaphoreValues(void)
{
    ReadSemaphores();

    printf("Mutex: %d\n", mutex_Value);
    printf("wrt: %d\n", wrt_Value);
    printf("Queue: %d\n", queue_Value);
}

void InitSemaphores(void)
{
    mutex = sem_open("prj3_1", O_RDWR, SEM_PERMS);
    SemaphoreCheck(mutex, "prj3_1");
    assert(mutex != SEM_FAILED);

    wrt = sem_open("prj3_2", O_RDWR, SEM_PERMS);
    SemaphoreCheck(wrt, "prj3_2");
    assert(wrt != SEM_FAILED);

    queue = sem_open("prj3_3", O_RDWR, SEM_PERMS);
    SemaphoreCheck(queue, "prj3_3");
    assert(queue != SEM_FAILED);
}

void PrintArrays(SharedStruct *ssx)
{
    int i;

    printf("Active Read Processes: ");
    for (i = 0; i < TABLESIZE; i++)
    {
        printf("\t%d", (int) ssx->ActiveReaders[i]);
    }
    printf("\n");

    printf("Active Write Processes: ");
    for (i = 0; i < TABLESIZE; i++)
    {
        printf("\t%d", (int) ssx->ActiveWriters[i]);
    }
    printf("\n");

    return;
}

void PrintNumbers(SharedStruct *ssx)
{
    printf("Readers completed: %d\n", ssx->ReadersCompleted);
    printf("Writers completed: %d\n", ssx->WritersCompleted);
    printf("Total records processed: %d\n", ssx->RecordsProcessed);

    return;
}

void PrintCounts(SharedStruct *ssx)
{
    printf("Active Readers Counter: %d\n", ssx->ActiveReadersCount);
    printf("Active Writers Counter: %d\n", ssx->ActiveWritersCount);
    return;
}