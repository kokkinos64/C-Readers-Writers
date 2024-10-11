#pragma once
#include <semaphore.h>
void MallocCheck(void *);
void SemaphoreCheck(sem_t *, char *);
int CreateSegment(void);
void *AttachSegment(int seg_id);
int DetachSegment(void *seg_pointer);
int RemoveSegment(int seg_id);