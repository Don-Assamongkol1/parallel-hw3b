#include <stdlib.h>

#include "output_module.h"
#include "packetsource.h"
#include "serial_module.h"
#include "types.h"

int main(int argc, char* argv[]) {
    if (argc != 6) {
        printf("Error! Expected 4 arguments: n, T, W trial_num, distribution type\n");
        return 0;
    }

    cmd_line_args_t* args = malloc(sizeof(cmd_line_args_t));  // commonly used variables
    args->T = atoi(argv[1]);                                  // number of threads; there are n - 1 workers ; recall argv[0] is ./<executable_name>
    args->n = atoi(argv[2]);                                  // number of packets from each source—(numPackets in the code)
    args->W = atoi(argv[3]);                                  // expected amount of work per packet—(mean in the code).
    args->distribution = argv[4][0];                          // distribution type either 'U', 'E'
    args->trial_num = atoi(argv[5]);                          // expected amount of work per packet—(mean in the code).
    args->numSources = args->n;

    if (args->distribution != 'U' && args->distribution != 'E') {
        printf("Error! distribution should be either 'U' or 'E' \n ");
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
    run_serial(packetSource, checksums_array, args);

    // clean up
    deletePacketSource(packetSource);

    return 0;
}