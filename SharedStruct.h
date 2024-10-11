#pragma once

#define TABLESIZE 25

struct shared_struct
{
    // Tables of active processes
    pid_t ActiveReaders[TABLESIZE];
    pid_t ActiveWriters[TABLESIZE];

    unsigned int ActiveReadersCount;
    unsigned int ActiveWritersCount;
    
    unsigned int ReadersCompleted;
    unsigned int WritersCompleted;
    unsigned int RecordsProcessed;

    double ReadersTimeSum_REAL;
    double WritersTimeSum_REAL;
    double MaxTimeToStart;
};

typedef struct shared_struct SharedStruct;

// void SharedStructInitializer(void); // For use only with the ALLOCATOR
void PrintTables(SharedStruct *s);
void AppendReader(SharedStruct *s, pid_t reader_id);
void RemoveReader(SharedStruct *s, pid_t reader_id);
void AppendWriter(SharedStruct *s, pid_t writer_id);
void RemoveWriter(SharedStruct *s, pid_t writer_id);
void SubmitTimeToStart(SharedStruct *s , double NewTime);