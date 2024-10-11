// This is the WRITER program

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
#include <sys/times.h>
#include "MallocCheck.h"
#include "SharedStruct.h"

#define SEM_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)

// Function prototypes
bool ScanArgumentsWRITER(int argc, char *argv[], char *DataFile, int *RecID, int *Value, int *ShmID, int *Time);
int getNumOfRecords(char *filename);
bool FileEditSingle(char *file_name, int RecordID, int TotalNumOfRecs, int ModVal);

// Semaphores
sem_t *mutex;
sem_t *wrt;
sem_t *queue;

int mutex_Value;
int write_Value;
int queue_Value;

int main(int argc, char *argv[])
{
    // Initialize the timer
    double t1, t2, cpu_time;
    struct tms tb1, tb2;
    double ticspersec;
    int i, sum = 0;
    ticspersec = (double)sysconf(_SC_CLK_TCK);
    t1 = (double)times(&tb1);

    // Variables
    int RecID;
    int Value;
    int ShmID;
    int Time;
    SharedStruct *mem_pointer;

    // Initialize random number generator
    srand(time(NULL));

    // Read the command-line arguments and check their validity
    char *DataFile = malloc(25 * sizeof(char));
    MallocCheck(DataFile);

    if (ScanArgumentsWRITER(argc, argv, DataFile, &RecID, &Value, &ShmID, &Time) == false)
    {
        exit(-1);
    }
    else
    {
        printf("Agruments given:\n \
        Data file: %s\n \
        RecID: %d\n \
        Value: %d\n \
        Shared Memory ID: %d\n \
        Time: %d\n",
               DataFile, RecID, Value, ShmID, Time);
    }

    // Initialze semaphores
    printf("Initializing named semaphores...\n");

    mutex = sem_open("prj3_1", O_RDWR, SEM_PERMS);
    SemaphoreCheck(mutex, "prj3_1");
    assert(mutex != SEM_FAILED);
    sem_getvalue(mutex, &mutex_Value);
    printf("Mutex: %d\n", mutex_Value);

    wrt = sem_open("prj3_2", O_RDWR, SEM_PERMS);
    SemaphoreCheck(wrt, "prj3_2");
    assert(wrt != SEM_FAILED);
    sem_getvalue(wrt, &write_Value);
    printf("wrt: %d\n", write_Value);

    queue = sem_open("prj3_3", O_RDWR, SEM_PERMS);
    SemaphoreCheck(wrt, "prj3_3");
    assert(queue != SEM_FAILED);
    sem_getvalue(queue, &queue_Value);
    printf("wrt: %d\n", queue_Value);

    // Attach the shared memory segment
    printf("WRITER %d: Attaching shared memory segment with ID %d...\n", getpid(), ShmID);
    mem_pointer = (SharedStruct *) AttachSegment(ShmID);

    // Get the total number of records in the file
    int NumOfRecs = getNumOfRecords(DataFile);

// =========[ ENTRY SECTION ]=========

    sem_wait(queue);    // DOWN, indicates that this writer is waiting in line to enter the CS
    sem_wait(wrt);      // DOWN, because I request access to write 

    // Notify the segment of your presence
    AppendWriter(mem_pointer, getpid());
    (mem_pointer->ActiveWritersCount)++;

    sem_post(queue);    // UP, to allow someone who's waiting to start working

// =========[ START OF CRITICAL SECTION ]=========

    t2 = (double)times(&tb2);
    SubmitTimeToStart(mem_pointer, (t2 - t1) / ticspersec);

    // WRITE
    FileEditSingle(DataFile, RecID, NumOfRecs, Value);

    // Sleep for a random number of records
    int sleepsecs = rand() % Time;
    printf("WRITER %d: Sleeping for %d seconds...\n", getpid(), sleepsecs);
    sleep(sleepsecs);

// =========[ END OF CRITICAL SECTION ]=========

// =========[ REMAINDER SECTION ]=========

    (mem_pointer->WritersCompleted)++; // Increase the number of writers that completed
    (mem_pointer->RecordsProcessed)++;   // Increase the total number of records processed
    (mem_pointer->ActiveWritersCount)--;
    RemoveWriter(mem_pointer, getpid());

    sem_post(wrt); // UP, unblock the next readers/writers who are waiting for their turn

// =================

    // Close the semaphores
    sem_close(mutex);
    sem_close(wrt);
    sem_close(queue);

    // Memory dislocation for cmd argument strings
    free(DataFile);

    // Update the timer
    t2 = (double)times(&tb2);
    // printf("WRITER %d: Run time was % lf sec (REAL time), although we used the CPU for % lf sec (CPU time).\n", (int) getpid(), (t2 - t1) / ticspersec, cpu_time / ticspersec);
    (mem_pointer->WritersTimeSum_REAL) += ( (t2 - t1) / ticspersec );

    return 0;
}

bool ScanArgumentsWRITER(int argc, char *argv[], char *DataFile, int *RecID, int *Value, int *ShmID, int *Time)
{
    int i;

    // Flags
    bool foundDataFile = false, foundRecID = false, foundValue = false, foundShmID = false, foundTime = false;

    // Scan for agruments
    for (i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "-f") == 0 && foundDataFile == false) // If you've found "-f", the next arg should be the DataFile
        {
            strcpy(DataFile, argv[i + 1]);
            foundDataFile = true;
        }

        if (strcmp(argv[i], "-l") == 0 && foundRecID == false) // If you've found "-l", the next arg should be the RecID
        {
            *RecID = atoi(argv[i + 1]);
            // strcpy(nocstr, argv[i + 1]);
            foundRecID = true;
        }

        if (strcmp(argv[i], "-v") == 0 && foundValue == false) // If you've found "-v", the next arg should be the Value
        {
            // strcpy(Value, argv[i + 1]);
            *Value = atoi(argv[i + 1]);
            foundValue = true;
        }

        if (strcmp(argv[i], "-s") == 0 && foundShmID == false) // If you've found "-i", the next arg should be the SharedMemoryID
        {
            *ShmID = atoi(argv[i + 1]);
            foundShmID = true;
        }

        if (strcmp(argv[i], "-d") == 0 && foundTime == false) // If you've found "-i", the next arg should be the Time
        {
            *Time = atoi(argv[i + 1]);
            foundTime = true;
        }
    }

    if (foundDataFile == false || foundRecID == false || foundValue == false || foundShmID == false || foundTime == false)
    {
        printf("Invalid arguments. Please, try again.\n");
        return false;
    }

    return true; // Args read sucessfully
}

int getNumOfRecords(char *filename)
{
    // Unused struct just to get its size
    // (may be different across different systems)
    typedef struct
    {
        int custid;
        char LastName[20];
        char FirstName[20];
        int balance;
    } MyRecord;

    // This code is based on the "Verify" program by A.Delis
    FILE *file_pointer;
    int nor = 0; // Number of total records in the file
    MyRecord rec;

    file_pointer = fopen(filename, "rb"); // Open the file for reading in binary mode

    if (file_pointer == NULL) // Check if file opening failed
    {
        printf("Cannot open binary file\n");
        exit(-1);
    }

    // Retrieve the number of records
    fseek(file_pointer, 0, SEEK_END);
    long lsize = ftell(file_pointer);
    nor = (int)lsize / sizeof(rec);

    fclose(file_pointer); // Close the file

    return nor;
}

bool FileEditSingle(char *file_name, int RecordID, int TotalNumOfRecs, int ModVal)
{
    // Unused struct just to get its size
    // (may be different across different systems)
    typedef struct
    {
        int custid;
        char LastName[20];
        char FirstName[20];
        int balance;
    } MyRecord;

    printf("WRITER %d: Writing to the record with ID %d...\n", getpid(), RecordID);

    // Open the file
    FILE *file_pointer;

    file_pointer = fopen(file_name, "r+b"); // Writing in binary mode
    if (file_pointer == NULL)
    {
        printf("WRITER %d: Cannot open the binary file.\n", getpid());
        return false;
    }

    // Memory allocation for the temp record to writed
    MyRecord *TempRec = malloc(sizeof(MyRecord));
    MallocCheck(TempRec);

    // CRITICAL SECTION
    for (int i = 0; i < TotalNumOfRecs; i++)
    {
        fread(TempRec, sizeof(MyRecord), 1, file_pointer);

        if (TempRec->custid == RecordID) // If we've found the account we were looking for,
        {
            printf("Customer ID: %d \nName: %s %s\nOld Balance: %d\n", TempRec->custid, TempRec->LastName, TempRec->FirstName, TempRec->balance);

            // Modify the balance according to cmd argument
            TempRec->balance += ModVal;

            printf("Modify by: %d\nNew balance: %d\n", ModVal, TempRec->balance);

            // and wrt it back to the binary file
            fseek(file_pointer, -sizeof(MyRecord), SEEK_CUR);   // Move the file cursor back to the beginning of this record
            fwrite(TempRec, sizeof(MyRecord), 1, file_pointer); // Replace the whole record there with our new, modified one

            break;
        }
    }

    // Memory dislocation for the temporary account
    free(TempRec);
}