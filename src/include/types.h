#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>
#include <unistd.h>

#include "packetsource.h"

typedef struct {
    int n;
    int T;
    int W;
    int trial_num;
    char distribution;
    int numSources;
} cmd_line_args_t;

typedef struct _queue {
    volatile int head;
    volatile int tail;
    volatile Packet_t** packet_array;
    int depth;
} queue_t;

#define DEPTH 32
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

#endif