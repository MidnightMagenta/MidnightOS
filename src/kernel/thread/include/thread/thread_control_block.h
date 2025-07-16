#ifndef MDOS_THREAD_CONTROL_BLOCK_H
#define MDOS_THREAD_CONTROL_BLOCK_H
#ifdef __cplusplus
extern "C" {
#endif

#define MAX_THREAD_COUNT 128

typedef struct{
    void* placeholer;
} ThreadControlBlock;

ThreadControlBlock tcb[MAX_THREAD_COUNT];

#ifdef __cplusplus
}
#endif
#endif