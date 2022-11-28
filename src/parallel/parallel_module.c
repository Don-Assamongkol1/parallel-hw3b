#include "parallel_module.h"

typedef struct _thread_args_t {
    int thread_idx;  // idx of the queue it should consume from + idx of where it adds checksum to
    long* checksums_array;
    queue_t** queues;
    int T;
    char strategy;
    int* source_to_num_packets_processed;
    int numSources;
} thread_args_t;

/* thr_func */
void* thr_func(void* input) {
    /* typecast arguments */
    thread_args_t* thr_args = (thread_args_t*)input;
    int thread_idx = thr_args->thread_idx;
    queue_t* thread_queue = thr_args->queues[thread_idx];
    char strategy = thr_args->strategy;

    /* Have worker process its queue while packets remain on them */
    int numPacketsProcessed = 0;
    while (numPacketsProcessed < thr_args->T) {
        volatile Packet_t* packet = malloc(sizeof(volatile Packet_t));

        if (strategy == 'L') {  // no lock needed
            while (true) {      // while true loop here prevents dequeuing from an empty queue
                if (dequeue(thread_queue, packet) == SUCCESS) {
                    break;
                }
            };
        }

        else if (strategy == 'H') {  // if strategy is homequeue then lock
            lock_lock(thread_queue->lock, thread_idx);
            while (true) {  // while true loop here prevents dequeuing from an empty queue
                if (dequeue(thread_queue, packet) == SUCCESS) {
                    break;
                }
            };
            lock_unlock(thread_queue->lock, thread_idx);
        }

        thr_args->checksums_array[thread_idx] += getFingerprint(packet->iterations, packet->seed);
        numPacketsProcessed += 1;
    }

    pthread_exit(NULL);
}

/* thr_func_awesome
 *
 *
 *
 *
 *
 *
 *
 *
 */
void* thr_func_awesome(void* input) {
    /* typecast arguments */
    thread_args_t* thr_args = (thread_args_t*)input;
    int thread_idx = thr_args->thread_idx;
    queue_t** queues = thr_args->queues;
    queue_t* home_queue = queues[thread_idx];
    int* source_to_num_packets_processed = thr_args->source_to_num_packets_processed;
    long* checksums_array = thr_args->checksums_array;
    int numSources = thr_args->numSources;

    while (true) {
        volatile Packet_t* packet = malloc(sizeof(volatile Packet_t));

        if (!queue_is_empty(home_queue)) {  // check if home queue can be worked on
            bool got_a_packet = false;

            lock_lock(home_queue->lock, thread_idx);
            if (!queue_is_empty(home_queue)) {
                source_to_num_packets_processed[thread_idx] += 1;  // indicate that we're going to process a packet from the source whose idx equals thread_idx
                got_a_packet = true;
                if (dequeue(home_queue, packet) == FAILURE) {
                    printf("you should never see this 1 \n");  // bc we already checked that it's non empty and we've acquired the lock, so no other thread should've been able to dequeue from this queue
                    pthread_exit(NULL);
                }
            }
            lock_unlock(home_queue->lock, thread_idx);

            if (got_a_packet) {
                checksums_array[thread_idx] += getFingerprint(packet->iterations, packet->seed);
            }
        }

        else {
            /* find a queue to help */
            int min_packets_processed = INT_MAX;
            int source_idx_to_help = -1;
            for (int i = 0; i < numSources; i++) {
                if (source_to_num_packets_processed[i] < min_packets_processed) {
                    min_packets_processed = source_to_num_packets_processed[i];
                    source_idx_to_help = i;
                }
            }

            if (min_packets_processed == thr_args->T) {  // the most behind source has had T packets processed already
                pthread_exit(NULL);
                // break;
            }

            queue_t* queue_to_help = queues[source_idx_to_help];

            if (!queue_is_empty(queue_to_help)) {  // check if this queue still needs help
                bool got_a_packet = false;

                lock_lock(queue_to_help->lock, thread_idx);
                if (!queue_is_empty(queue_to_help)) {
                    source_to_num_packets_processed[source_idx_to_help] += 1;
                    got_a_packet = true;
                    if (dequeue(queue_to_help, packet) == FAILURE) {
                        printf("you should never see this 2 \n");  // bc we already checked that its non empty and we have the lock
                        pthread_exit(NULL);
                    }
                }
                lock_unlock(queue_to_help->lock, thread_idx);

                if (got_a_packet) {
                    checksums_array[source_idx_to_help] += getFingerprint(packet->iterations, packet->seed);
                }
            }
        }
    }

    pthread_exit(NULL);
}

/* run_parallel main function
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */
int run_parallel(PacketSource_t* packetSource, long* checksums_array, cmd_line_args_t* args) {
    StopWatch_t* stopwatch = malloc(sizeof(StopWatch_t));
    startTimer(stopwatch);

    /* Create numSources many queues */
    queue_t* queues[args->numSources];
    for (int i = 0; i < args->numSources; i++) {
        queues[i] = create_queue();

        if (args->lock_type == '3') {
            queues[i]->lock->locktype = TAS_LOCK_TYPE;
        } else if (args->lock_type == '4') {
            queues[i]->lock->locktype = TTAS_LOCK_TYPE;
        }
    }

    /* Create source_to_num_packets_processed variable for the 'A'-awesome strategy */
    int source_to_num_packets_processed[args->numSources];
    for (int i = 0; i < args->numSources; i++) {
        source_to_num_packets_processed[i] = 0;
    }

    /* spawn n - 1 worker threads */
    int numThreads = args->numSources;
    thread_args_t thr_args[numThreads];  // must memory allocate the arg to each thread
    pthread_t thread_ids[numThreads];    // keep track of our threads

    for (int i = 0; i < numThreads; i++) {
        thr_args[i].thread_idx = i;
        thr_args[i].checksums_array = checksums_array;
        thr_args[i].queues = queues;
        thr_args[i].T = args->T;
        thr_args[i].strategy = args->strategy;
        thr_args[i].source_to_num_packets_processed = source_to_num_packets_processed;
        thr_args[i].numSources = args->numSources;

        // Spawn different thread functions based on our strategy; makes the thread func easier to read
        if (args->strategy == 'A') {
            if (pthread_create(&(thread_ids[i]), NULL, &thr_func_awesome, (void*)&thr_args[i]) != 0) {
                printf("error creating thread!\n");
                return 1;
            }
        } else {
            if (pthread_create(&(thread_ids[i]), NULL, &thr_func, (void*)&thr_args[i]) != 0) {
                printf("error creating thread!\n");
                return 1;
            }
        }
    }

    // Have dispatcher go through and put packets onto the queue
    for (int packetIndex = 0; packetIndex < args->T; packetIndex++) {
        for (int sourceNum = 0; sourceNum < args->numSources; sourceNum++) {
            volatile Packet_t* packet = NULL;

            if (args->distribution == 'U') {
                packet = getUniformPacket(packetSource, sourceNum);
            } else if (args->distribution == 'E') {
                packet = getExponentialPacket(packetSource, sourceNum);
            }

            while (true) {  // this while loop prevents us from enqueueing to a full queue
                if (enqueue(queues[sourceNum], packet) == SUCCESS) {
                    break;
                }
            };
        }
    }
    // printf("done enqueueing all packets from dispatcher\n");

    /* Join our threads */
    for (int i = 0; i < numThreads; i++) {
        if (pthread_join(thread_ids[i], NULL) != 0) {
            printf("error joining thread!\n");
            return 1;
        }
    }

    /* Free memory for the queues and locks we used */
    for (int i = 0; i < args->numSources; i++) {
        lock_destroy(queues[i]->lock);
        free(queues[i]);
    }

    stopTimer(stopwatch);
    double elapsed_time = getElapsedTime(stopwatch);
    printf("elapsed_time: %f\n", elapsed_time);
    free(stopwatch);

    return EXIT_SUCCESS;
}