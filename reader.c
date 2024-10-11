// This is the READER program

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
#include "SharedStruct.h"
#include "MallocCheck.h"

#define SEM_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)

// Function prototypes
bool ScanArgumentsREADER(int argc, char *argv[], char *DataFile, int *RecID, int *RangeStart, int *RangeEnd, int *Time, int *ShmID /*, char *nocstr*/);
bool FileReader(char *file_name, unsigned int AccountsToRead, unsigned int TotalNumOfRecs, double *balance_average);
bool FileReaderSingle(char *file_name, int RecordID, int TotalNumOfRecs); // Read just one record with a specific ID
int getNumOfRecords(char *filename);

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

    // Variables for shared mem
    int segment_id = 0;
    SharedStruct *mem_pointer;

    // Other variables
    int RecID, range_start, range_end, Time;

    // Initialize random number generator
    srand(time(NULL));

    // Memory allocation for cmd argument strings
    char *DataFile = malloc(30 * sizeof(char));
    MallocCheck(DataFile);

    // Read the command-line arguments and check their validity
    if (ScanArgumentsREADER(argc, argv, DataFile, &RecID, &range_start, &range_end, &Time, &segment_id) == false)
    {
        exit(-1);
    }
    else
    {
        printf("\033[0;36m");
        printf("Agruments given:\n Data file: %s\n RecID: %d\n RangeStart: %d\n RangeEnd: %d\n Time: %d\n Shared Memory ID: %d\n", DataFile, RecID, range_start, range_end, Time, segment_id);
        printf("\033[0m");
    }

    // Initialze semaphores
    printf("Initializing named semaphores...\n");

    mutex = sem_open("prj3_1", O_RDWR, SEM_PERMS);
    SemaphoreCheck(mutex, "prj3_1");
    assert(mutex != SEM_FAILED);
    sem_getvalue(mutex, &mutex_Value);
    printf("Number of active readers: %d\n", mutex_Value);

    wrt = sem_open("prj3_2", O_RDWR, SEM_PERMS);
    SemaphoreCheck(wrt, "prj3_2");
    assert(wrt != SEM_FAILED);
    sem_getvalue(wrt, &write_Value);
    printf("Number of active writers: %d\n", write_Value);

    queue = sem_open("prj3_3", O_RDWR, SEM_PERMS);
    SemaphoreCheck(wrt, "prj3_3");
    assert(queue != SEM_FAILED);
    sem_getvalue(queue, &queue_Value);
    printf("Write: %d\n", queue_Value);

    // Attach the shared memory segment
    printf("READER %d: Attaching shared memory segment with ID %d...\n", getpid(), segment_id);

    mem_pointer = (SharedStruct *)shmat(segment_id, (void *)0, 0);

    if (*(int *)mem_pointer == -1) // Check if attachment was unsucessful
    {
        perror("Attachement");
    }
    else
    {
        printf("READER %d: Successfully attached!\n", getpid());
    }

    // Read and print statistics
    int NumOfRecords = getNumOfRecords(DataFile);
    double balance_avg;

    // if (sem_wait(wrt) < 0)
    // {
    //     printf("READER %d: Writers are present, unable to read.\n", getpid());
    // }

    // You're the reader.
    // If there is an active writer, do not enter, wait for him to finish writing and then check again.
    


// =========[ ENTRY SECTION ]=========

    sem_wait(queue);    // DOWN, indicates that this reader is waiting in line to enter the CS
    sem_wait(mutex);    // DOWN, to prohibit the ActiveReadersCount from being modified

    // Update the segment for your arrival
    AppendReader(mem_pointer, getpid());
    (mem_pointer->ActiveReadersCount)++;

    if (mem_pointer->ActiveReadersCount == 1)   // If I am the first reader (and there's nobody else)
    {
        sem_wait(wrt); // DOWN, block the writers
    }

    sem_post(queue);    // UP, to allow the next reader/writer who's waiting in line to start working
    sem_post(mutex);    // UP, to allow the ActiveReadersCount to be modified
    

// =========[ START OF CRITICAL SECTION ]=========

    t2 = (double)times(&tb2);
    SubmitTimeToStart(mem_pointer, (t2 - t1) / ticspersec);

    // READ
    if (RecID == 0) // Read a range of records
    {
        FileReader(DataFile, range_end - range_start, NumOfRecords, &balance_avg);
        printf("READER %d: The average balance of these %d accounts is $%lf.\n", getpid(), range_end - range_start, balance_avg);
        (mem_pointer->RecordsProcessed) += (range_end - range_start); // Increase the total number of records processed
    }
    else
    {
        FileReaderSingle(DataFile, RecID, NumOfRecords);
        (mem_pointer->RecordsProcessed)++; // Increase the total number of records processed
    }

    // Sleep for a random number of seconds (max defined by cmd)
    int sleepsecs = rand() % Time;
    printf("READER %d: Sleeping for %d seconds...\n", getpid(), sleepsecs);
    sleep(sleepsecs);

// =========[ END OF CRITICAL SECTION ]=========

// =========[ REMAINDER SECTION ]=========

    sem_wait(mutex); // DOWN, to once again prohibit the ActiveReadersCount from being modified

    // Update the segment for your departure
    (mem_pointer->ReadersCompleted)++;   // Increase the number of readers that completed
    (mem_pointer->ActiveReadersCount)--; // Decrease the number of active readers
    RemoveReader(mem_pointer, getpid()); // Remove PID from ActiveReaders array  

    if ( (mem_pointer->ActiveReadersCount) == 0) // Now, if ALL the readers are done reading
    {
        sem_post(wrt); // UP, you have to announce it to the writers, thus UNblockthem
    }

    sem_post(mutex); // UP, to allow the ActiveReadersCount to be modified again

// ==================

    // Close the semaphores
    sem_close(mutex);
    sem_close(wrt);
    sem_close(queue);

    // Memory dislocation for cmd argument strings
    free(DataFile);

    // Update the timer
    t2 = (double)times(&tb2);
    // printf("READER %d: Run time was % lf sec (REAL time), although we used the CPU for % lf sec (CPU time).\n", (int) getpid(), (t2 - t1) / ticspersec, cpu_time / ticspersec);
    (mem_pointer->ReadersTimeSum_REAL) += ( (t2 - t1) / ticspersec );

    return 0;
}

bool ScanArgumentsREADER(int argc, char *argv[], char *DataFile, int *RecID, int *RangeStart, int *RangeEnd, int *Time, int *ShmID /*, char *nocstr*/)
{
    int i;

    // Flags
    bool foundDataFile = false, foundRecID = false, foundTime = false, foundShmID = false;

    // Scan for DataFile
    for (i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "-f") == 0 && foundDataFile == false) // If you've found "-f", the next arg should be the DataFile
        {
            strcpy(DataFile, argv[i + 1]);
            foundDataFile = true;
        }

        if (strcmp(argv[i], "-l") == 0 && foundRecID == false) // If you've found "-l", the next arg should be the RecID
        {
            char *position_of_comma = strchr(argv[i + 1], ','); // Check if the string includes a comma (if yes, we have a range)

            if (position_of_comma == NULL) // If the comma is not found, we have just a single record (and not a range of records)
            {
                *RecID = atoi(argv[i + 1]);
            }
            else
            {
                sscanf(argv[i + 1], "%d,%d", RangeStart, RangeEnd);
            }

            // strcpy(nocstr, argv[i + 1]);
            foundRecID = true;
        }

        if (strcmp(argv[i], "-d") == 0 && foundTime == false) // If you've found "-d", the next arg should be the Time
        {
            // strcpy(Time, argv[i + 1]);
            *Time = atoi(argv[i + 1]);
            foundTime = true;
        }

        if (strcmp(argv[i], "-s") == 0 && foundShmID == false) // If you've found "-i", the next arg should be the SharedMemoryID
        {
            *ShmID = atoi(argv[i + 1]);
            foundShmID = true;
        }
    }

    if (foundDataFile == false || foundRecID == false || foundTime == false || foundShmID == false)
    {
        printf("Invalid arguments. Please, try again.\n");
        return false;
    }

    return true; // Args read sucessfully
}

bool FileReader(char *file_name, unsigned int AccountsToRead, unsigned int TotalNumOfRecs, double *balance_average)
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

    // Variables for statistics
    *balance_average = 0;
    int balance_sum = 0;

    printf("READER %d: Reading %d (out of %d) accounts from %s...\n", getpid(), AccountsToRead, TotalNumOfRecs, file_name);

    // Open the file
    FILE *file_pointer;

    file_pointer = fopen(file_name, "rb"); // Reading in binary mode
    if (file_pointer == NULL)
    {
        printf("READER %d: Cannot open the binary file.\n", getpid());
        return false;
    }

    // Memory allocation for the temp record to read
    MyRecord *TempRec = malloc(sizeof(MyRecord));
    MallocCheck(TempRec);

    // CRITICAL SECTION

    for (int i = 0; i < AccountsToRead; i++)
    {
        fread(TempRec, sizeof(MyRecord), 1, file_pointer);

        balance_sum += TempRec->balance; // Sum

        printf("%d %s %s  %d \n",
               TempRec->custid, TempRec->LastName,
               TempRec->FirstName, TempRec->balance);
    }

    // Memory dislocation for the temporary account
    free(TempRec);

    // Calculate the average balance
    *balance_average = (double)balance_sum / (double)AccountsToRead;

    return true;
}

bool FileReaderSingle(char *file_name, int RecordID, int TotalNumOfRecs) // Read just one record with a specific ID
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

    printf("READER %d: Reading the record with ID %d...\n", getpid(), RecordID);

    // Open the file
    FILE *file_pointer;

    file_pointer = fopen(file_name, "rb"); // Reading in binary mode
    if (file_pointer == NULL)
    {
        printf("READER %d: Cannot open the binary file.\n", getpid());
        return false;
    }

    // Memory allocation for the temp record to read
    MyRecord *TempRec = malloc(sizeof(MyRecord));
    MallocCheck(TempRec);

    // CRITICAL SECTION
    for (int i = 0; i < TotalNumOfRecs; i++)
    {
        fread(TempRec, sizeof(MyRecord), 1, file_pointer);

        if (TempRec->custid == RecordID) // If we've found the account we were looking for,
        {
            // Just print it!
            printf("%d %s %s  %d \n",
                   TempRec->custid, TempRec->LastName,
                   TempRec->FirstName, TempRec->balance);
        }
    }

    // Memory dislocation for the temporary account
    free(TempRec);
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