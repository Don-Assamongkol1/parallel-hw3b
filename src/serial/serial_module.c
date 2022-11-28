#include "serial_module.h"

void run_serial(PacketSource_t* packetSource, long* checksums_array, cmd_line_args_t* args) {
    StopWatch_t* stopwatch = malloc(sizeof(StopWatch_t));
    startTimer(stopwatch);

    for (int packetNum = 0; packetNum < args->T; packetNum++) {
        for (int sourceNum = 0; sourceNum < args->numSources; sourceNum++) {
            volatile Packet_t* packet = NULL;

            if (args->distribution == 'U') {
                packet = getUniformPacket(packetSource, sourceNum);
            } else if (args->distribution == 'E') {
                packet = getExponentialPacket(packetSource, sourceNum);
            }

            checksums_array[sourceNum] += getFingerprint(packet->iterations, packet->seed);
        }
    }

    stopTimer(stopwatch);
    double elapsed_time = getElapsedTime(stopwatch);
    printf("elapsed_time: %f\n", elapsed_time);
    free(stopwatch);
}
