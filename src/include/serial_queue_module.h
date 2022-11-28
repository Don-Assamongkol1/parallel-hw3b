#include "fingerprint.h"
#include "packetsource.h"
#include "queue.h"
#include "stopwatch.h"
#include "types.h"

void run_serial_queue(PacketSource_t* packetSource, long* checksums_array, cmd_line_args_t* args);