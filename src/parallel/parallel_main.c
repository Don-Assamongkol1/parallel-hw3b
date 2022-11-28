#include <stdio.h>
#include <stdlib.h>

#include "output_module.h"
#include "packetsource.h"
#include "parallel_module.h"
#include "stopwatch.h"
#include "types.h"

int main(int argc, char* argv[]) {
    if (argc != 6) {
        printf("Error! Expected more arguments \n");
        return 0;
    }

    cmd_line_args_t* args = malloc(sizeof(cmd_line_args_t));  // commonly used variables
    args->T = atoi(argv[1]);                                  // number of threads; there are n - 1 workers ; recall argv[0] is ./<executable_name>
    args->n = atoi(argv[2]);                                  // number of packets from each source—(numPackets in the code)
    args->W = atoi(argv[3]);                                  // expected amount of work per packet—(mean in the code).
    args->distribution = argv[4][0];                          // distribution type either 'U', 'E'
    args->trial_num = atoi(argv[5]);                          // expected amount of work per packet—(mean in the code).
    args->lock_type = argv[6][0];                             // '3' for TAS, '4' for TTAS
    args->strategy = argv[7][0];                              // ‘L’, ‘H’, ‘A’

    args->numSources = args->n - 1;

    if (args->distribution != 'U' && args->distribution != 'E') {
        printf("Error! distribution should be either 'U' or 'E' \n ");
        return 0;
    }

    if (args->lock_type != '3' && args->lock_type != '4') {
        printf("Error! lock_type should be either '3' (for TAS) or '4' (for TTAS) \n ");
        return 0;
    }

    if (args->strategy != 'L' && args->strategy != 'H' && args->strategy != 'A') {
        printf("Error! strategy should be either 'L' (lock-free), 'H' (HomeQueue) or 'A' (Awesome) \n ");
        return 0;
    }

    // create our packet source
    PacketSource_t* packetSource = createPacketSource((long)args->W, args->numSources, (short)args->trial_num);

    // create our checksums array, where we store the checksum for each source
    long checksums_array[args->numSources];
    for (int i = 0; i < args->numSources; i++) {
        checksums_array[i] = 0;
    }

    // single-threaded: have our thread grab T packets from each source and compute their checksum
    run_parallel(packetSource, checksums_array, args);

    // clean up
    deletePacketSource(packetSource);

    return 0;
}