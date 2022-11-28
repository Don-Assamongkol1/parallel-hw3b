#include "fingerprint.h"
#include "pthread.h"
#include "queue.h"
#include "stopwatch.h"
#include "types.h"

int run_parallel(PacketSource_t* packetSource, long* checksums_array, cmd_line_args_t* args);