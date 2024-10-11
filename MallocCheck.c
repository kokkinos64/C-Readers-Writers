// This is a collection of all the useful shared memory operations with their own checks.

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
// #include <semaphore.h>
#include <assert.h>
#include <errno.h>
#include "MallocCheck.h"

void MallocCheck(void *s)
{
    if (s == NULL)
    {
        printf("Error in malloc check!\n");
        printf("Address %p is NULL!\n", &s);
        // exit(-1);
        return;
    }
    else
    {
        return; // malloc successful
    }
}

void SemaphoreCheck(sem_t *sem, char *sem_name)
{
    if (sem != SEM_FAILED)
    {
        printf("Created new semaphore.\n");
    }
    else if (errno == EEXIST)
    {
        printf("Semaphore appears to already exist. Attempting to open...\n");
        sem = sem_open(sem_name, 0);
    }
    else
    {
        ;
    }

    return;   
}

int CreateSegment(void)
{
    int seg_id;

    seg_id = shmget(IPC_PRIVATE, 1, 0666);

    if (seg_id == -1) // Check if creation was unsuccesful
    {
        perror("Creation");
    }
    else
    {
        printf("Allocated. Shared segment's ID is %d.\n", (int)seg_id);
    }

    return seg_id;
}

void *AttachSegment(int seg_id)
{
    void *seg_pointer;

    seg_pointer = shmat(seg_id, (void *)0, 0);

    if ( (void *) seg_pointer == -1)
    {
        perror("Attachment");
    }
    else
    {
        printf("Attached successfully.\n");
        // printf("CHAR: %c\n", *pointer);
    }

    return seg_pointer;
}

int DetachSegment(void *seg_pointer)
{
    int error_code;

    error_code = shmdt((void *) seg_pointer);

    // Check if detachment was sucessful
    if (error_code == -1)
    {
        perror("Detachement");
    }
    else
    {
        printf("Detached! Error code: %d\n", error_code);
    }

    return error_code;
}

int RemoveSegment(int seg_id)
{
    int error_code;

    error_code = shmctl(seg_id, IPC_RMID, 0); 

    if (error_code == -1)
    {
        perror("Removal");
    }
    else
    {
        printf("Removed. Error code: %d\n", (int) error_code);
    }

    return error_code;
}