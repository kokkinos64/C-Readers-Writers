#include <stdio.h>
#include <stdlib.h>
#include "SharedStruct.h"
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "MallocCheck.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include "MallocCheck.h"

// Hidden prototypes
bool DoesReaderExist(SharedStruct *s, pid_t reader_id);
bool DoesWriterExist(SharedStruct *s, pid_t writer_id);

void PrintTables(SharedStruct *s)
{
    int i;

    printf("Active readers: ");
    for (i = 0; i < TABLESIZE; i++)
    {
        printf("%d\t", (int)s->ActiveReaders[i]);
    }
    printf("\n");

    printf("Active writers: ");
    for (i = 0; i < TABLESIZE; i++)
    {
        printf("%d\t", (int)s->ActiveWriters[i]);
    }
    printf("\n");

    printf("Readers who completed their job: %d\n", s->ReadersCompleted);
    printf("Writers who completed their job %d\n", s->WritersCompleted);
    printf("Total records processed: %d\n", s->RecordsProcessed);

    return;
}

// void SharedStructInitializer(void) // For use only with the ALLOCATOR
// {
//     // Memory allocation for the struct itself
//     SharedStruct *Instance;
//     Instance = malloc(sizeof(SharedStruct)); MallocCheck(Instance);

//     // Memory allocation for the PID tables (active readers and writers)
//     Instance->ActiveReaders = malloc(10 * sizeof(pid_t)); MallocCheck(Instance->ActiveReaders);
//     Instance->ActiveWriters = malloc(10 * sizeof(pid_t));  MallocCheck(Instance->ActiveWriters);

//     Instance->Babis = malloc(25 * sizeof(char)); MallocCheck(Instance->Babis);

//     // Initialization of the semaphores
//     // >?????

//     return;
// }

// ==========================================================================================
// ==========================================================================================
// ==========================================================================================
// ==========================================================================================


bool DoesReaderExist(SharedStruct *s, pid_t reader_id)
{
    int i;
    bool isFound = false;

    for (i = 0; i < TABLESIZE; i++)
    {
        if (s->ActiveReaders[i] == reader_id)
        {
            isFound = true;
            break;
        }
    }

    return isFound;
}

void AppendReader(SharedStruct *s, pid_t reader_id)
{
    int i;

    if (DoesReaderExist(s, reader_id) == true) // First, find out if this reader is already present
    {
        printf("The reader with PID %d is already present.\n", (int)reader_id);
    }
    else // If not,
    {
        for (i = 0; i < TABLESIZE; i++) // Scan the whole array
        {
            if (s->ActiveReaders[i] == 0) // If you find an empty slot
            {
                s->ActiveReaders[i] = reader_id; // Place the PID of the reader there
                break;
            }
        }
    }
    printf("READER %d INSERTED INTO ARRAY.\n", (int)reader_id);
    return;
}

void RemoveReader(SharedStruct *s, pid_t reader_id)
{
    int i;

    // if (DoesReaderExist(s, reader_id) == true) // First, find out if this reader is already present
    // {
    //     printf("The reader with PID %d is already present.\n", (int) reader_id);
    // }
    // else // If not,
    // {
    for (i = 0; i < TABLESIZE; i++) // Scan the whole array
    {
        if (s->ActiveReaders[i] == reader_id) // If you find the reader's PID
        {
            s->ActiveReaders[i] = 0; // Mark it as empty
            break;
        }
    }
    // }

    printf("READER %d REMOVED FROM ARRAY.\n", (int)reader_id);
    return;
}

// ==========================================================================================
// ==========================================================================================
// ==========================================================================================
// ==========================================================================================


bool DoesWriterExist(SharedStruct *s, pid_t writer_id)
{
    int i;
    bool isFound = false;

    for (i = 0; i < TABLESIZE; i++)
    {
        if (s->ActiveWriters[i] == writer_id)
        {
            isFound = true;
            break;
        }
    }

    return isFound;
}

void AppendWriter(SharedStruct *s, pid_t writer_id)
{
    int i;

    if (DoesWriterExist(s, writer_id) == true) // First, find out if this writer is already present
    {
        printf("The writer with PID %d is already present.\n", (int)writer_id);
    }
    else // If not,
    {
        for (i = 0; i < TABLESIZE; i++) // Scan the whole array
        {
            if (s->ActiveWriters[i] == 0) // If you find an empty slot
            {
                s->ActiveWriters[i] = writer_id; // Place the PID of the writer there
                break;
            }
        }
    }
    printf("WRITER %d INSERTED INTO ARRAY.\n", (int)writer_id);
    return;
}

void RemoveWriter(SharedStruct *s, pid_t writer_id)
{
    int i;

    for (i = 0; i < TABLESIZE; i++) // Scan the whole array
    {
        if (s->ActiveWriters[i] == writer_id) // If you find the writer's PID
        {
            s->ActiveWriters[i] = 0; // Mark it as empty
            break;
        }
    }

    printf("WRITER %d REMOVED FROM ARRAY.\n", (int)writer_id);
    return;
}

// =================

void SubmitTimeToStart(SharedStruct *s , double NewTime)
{   
    printf("Current time: %lf, Max: %lf\n", NewTime, s->MaxTimeToStart);
    if(s->MaxTimeToStart < NewTime)
    {
        s->MaxTimeToStart = NewTime;
        printf("New Max applied.\n");
    }

    return;
}