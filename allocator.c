// This is the ALLOCATOR program

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
#include <errno.h>

#include "MallocCheck.h"
#include "SharedStruct.h"

#define SEM_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)

// Semaphores
sem_t *mutex;
sem_t *wrt;
sem_t *queue; // Controlling the readers

int main(void)
{
    printf("This is the allocator program.\n");

    // Variables
    int segment_id = 0, error_code = 0;
    SharedStruct *mem_pointer;

    // Initialze semaphores
    printf("Creating semaphores...");
    mutex = sem_open(/*Semaphore name */ "prj3_1", /* Flag */ O_CREAT, /* Permissions */ SEM_PERMS, /* Initial value*/ 1);
    wrt = sem_open(/*Semaphore name */ "prj3_2", /* Flag */ O_CREAT, /* Permissions */ SEM_PERMS, /* Initial value*/ 1);
    queue = sem_open(/*Semaphore name */ "prj3_3", /* Flag */ O_CREAT, /* Permissions */ SEM_PERMS, /* Initial value*/ 1);
    printf("Done.\n");

    // Create the shared memory segment
    printf("Creating shared segment...");
    segment_id = shmget(IPC_PRIVATE, 10, 0666);

    if (segment_id == -1) // Check if creation was unsuccesful
    {
        perror("Creation");
    }
    else
    {
        printf("Allocated. \nShared segment's ID is %d.\n", (int)segment_id);
    }

    // Attach the segment
    mem_pointer = (SharedStruct *)shmat(segment_id, (void *)0, 0);

    if ((SharedStruct *)mem_pointer == -1)
    {
        perror("Attachment");
    }
    else
    {
        printf("Segment can be attached successfully.\n");
        // printf("Memory contents: \n");
        // PrintStruct(mem_pointer);
    }

    // Initialize EVERYTHING!
    mem_pointer->ActiveReadersCount = 0;
    mem_pointer->ActiveWritersCount = 0;
    mem_pointer->MaxTimeToStart = 0;
    mem_pointer->ReadersTimeSum_REAL = 0;
    mem_pointer->WritersTimeSum_REAL = 0;
    mem_pointer->ReadersCompleted = 0;
    mem_pointer->WritersCompleted = 0;
    mem_pointer->RecordsProcessed = 0;
    
    printf("You can now initiate other processes. Press any button to terminate.");
    getchar();

    // Print statistics
    printf("\n===========================[ STATISTICS ]===========================\n");
    printf("Total readers that worked with the file: %d\n", mem_pointer->ReadersCompleted);
    printf("Average active time of readers: %lf seconds\n", mem_pointer->ReadersTimeSum_REAL / mem_pointer->ReadersCompleted);
    printf("Total writers that worked with the file: %d\n", mem_pointer->WritersCompleted);
    printf("Average active time of writers: %lf seconds\n", mem_pointer->WritersTimeSum_REAL / mem_pointer->WritersCompleted);
    printf("Maximum delay: %lf seconds\n", mem_pointer->MaxTimeToStart);
    printf("Total number of records processed: %d\n", mem_pointer->RecordsProcessed);
    printf("====================================================================\n\n");


    // Remove the segment
    printf("Do you want to dislocate the shared memory segment? (y/n): ");
    if (getchar() == 'y')
    {
        // Remove the segment
        error_code = shmctl(segment_id, IPC_RMID, 0);

        if (error_code == -1)
        {
            perror("Removal");
        }
        else
        {
            printf("Segment %d was successfully removed.\n", segment_id);
        }

        // Destrorying Semaphores
        printf("Destroying semaphores...");
        sem_destroy(mutex);
        sem_destroy(wrt);
        sem_destroy(queue);
        printf("Done.\n");
    }
    else
    {
        printf("Shared memory segment %d is still live.\n", segment_id);

        // Closing Semaphores
        printf("Closing semaphores...");
        sem_close(mutex);
        sem_close(wrt);
        sem_close(queue);
        printf("Done.\n");
    }

    return 0;
}