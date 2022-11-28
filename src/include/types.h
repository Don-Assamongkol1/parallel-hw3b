#ifndef TYPES_H
#define TYPES_H

#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "packetsource.h"

/******************* DEFINITIONS FROM PROJECT 3a **********************/
#define DEPTH 8
#define SLEEP_DURATION 100  // to make queue full/empty for stress testing
// this is in microseconds i.e. 10^-6

/* Return codes for queue operations */
#define SUCCESS 0
#define FAILURE 1

/* program codes to name output files */
#define SERIAL 110
#define PARALLEL 111
#define SERIAL_QUEUE 112

/* Writing to output */
#define MAX_LINE_LENGTH 100000
#define MAX_STRING_LENGTH 100

#define BIG 10000000
#define TAS_LOCK_TYPE 0
#define PTHREAD_LOCK_TYPE 1  // Note: we're just wrapping the default pthreads given to us
#define A_LOCK_TYPE 2
#define TTAS_LOCK_TYPE 3

#define UNUSED_ARG 10000  // pass into our calls to the lock method; only a_lock needs a thread id

typedef struct {
    volatile int state;  // 0 is false, 1 is true
} tas_lock_t;

typedef struct {
    volatile int* flags_array;
    volatile int tail;
    int numThreads;          // the maximum number of threads that will call our lock
    int* thread_id_to_slot;  // map thread id's to their slot in our flags array
} a_lock_t;

typedef struct {
    pthread_mutex_t* mutex;
} pthread_lock_t;

typedef struct {
    volatile int state;  // 0 is false, 1 is true
} ttas_lock_t;

typedef union {
    tas_lock_t* tas_lock;
    a_lock_t* a_lock;
    pthread_lock_t* pthread_lock;
    ttas_lock_t* ttas_lock;
} __lock_t;

typedef struct {  // our wrapper class
    int locktype;
    int numThreads;
    __lock_t* lock;
} lock_t;

/******************* DEFINITIONS FROM PROJECT 2 **********************/
typedef struct {
    int T;
    int n;
    int W;
    char distribution;
    int trial_num;
    char lock_type;  // '3' or '4'
    char strategy;   // 'L', 'H', or 'A'

    int numSources;  // derived field from the passed in cmd line args
} cmd_line_args_t;

typedef struct {
    volatile int head;
    volatile int tail;
    volatile Packet_t** packet_array;
    int depth;
    lock_t* lock;
} queue_t;

/******************* DEFINITIONS FROM PROJECT 3b **********************/

#endif
