// This is the DISLOCATOR program

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "MallocCheck.h"
#include <sys/ipc.h>
#include <sys/shm.h>

#define SEM_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)


// Semaphores
sem_t *mutex;
sem_t *writesem;
sem_t *queue;

int main(int argc, char **argv)
{
    printf("This is the dislocator program.\n");

    int segment_id = 0, error_code = 0;

    // Initialze semaphores
    printf("Reading the semaphores...");
    mutex = sem_open("prj3_1", O_RDWR, SEM_PERMS);
    writesem = sem_open("prj3_2", O_RDWR, SEM_PERMS);
    queue = sem_open("prj3_3", O_RDWR, SEM_PERMS);
    printf("Done.\n");

    // Read the ID
    if (argc <= 1) // Shared memory ID was not in cmd args
    {
        printf("Please enter  the shared memory ID: ");
        scanf("%d", &segment_id);
    }
    else
    {
        sscanf(argv[1], "%d", &segment_id); // Read ID from argv[1]
    }

    // Remove the segment
    error_code = shmctl(segment_id, IPC_RMID, 0);

    if (error_code == -1)
    {
        perror("Removal");
    }
    else
    {
        printf("Shared memory segment %d was successfully REMOVED.\n", segment_id);
    }

    // Destrorying Semaphores
    printf("Destroying semaphores...");
    sem_destroy(mutex);
    sem_destroy(writesem);
    sem_destroy(queue);
    printf("Done.\n");

    return 0;
}