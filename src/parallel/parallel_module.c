#include "parallel_module.h"

typedef struct _thread_args_t {
    int thread_idx;  // idx of the queue it should consume from + idx of where it adds checksum to
    long* checksums_array;
    queue_t** queues;
    int T;
    char strategy;
    int* source_to_num_packets_processed;
    int numSources;
    long** packetSignatures;
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
    int home_idx = thread_idx;
    queue_t** queues = thr_args->queues;

    int* source_to_num_packets_processed = thr_args->source_to_num_packets_processed;
    long* checksums_array = thr_args->checksums_array;
    int numSources = thr_args->numSources;
    long** packetSignatures = thr_args->packetSignatures;

    while (true) {
        volatile Packet_t* packet = malloc(sizeof(volatile Packet_t));

        if (!queue_is_empty(queues[home_idx])) {  // check if home queue can be worked on
            bool got_a_packet = false;            // variable: did this thread manage to get a packet to work on?

            lock_lock(queues[home_idx]->lock, thread_idx);
            if (!queue_is_empty(queues[home_idx])) {
                source_to_num_packets_processed[home_idx] += 1;  // indicate that we're going to process a packet from the source whose idx equals thread_idx
                got_a_packet = true;
                if (dequeue(queues[home_idx], packet) == FAILURE) {
                    printf("you should never see this 1 \n");  // if we've locked the queue and then checked that it's non-empty,
                    // pthread_exit(NULL);                        // then between then and now, no thread should've dequeued from this queue
                    got_a_packet = false;
                }
            }
            lock_unlock(queues[home_idx]->lock, thread_idx);

            if (got_a_packet) {
                long int packet_checksum = getFingerprint(packet->iterations, packet->seed);
                lock_lock(queues[home_idx]->lock, thread_idx);  // conceptually we want to lock access to checksums_array[thread_idx]. We're using the queue's lock to doubly represent this
                checksums_array[home_idx] += packet_checksum;

                packetSignatures[home_idx][source_to_num_packets_processed[home_idx]] = packet_checksum;

                lock_unlock(queues[home_idx]->lock, thread_idx);
            }
        }

        else {                                             /* find a queue to help */
            int min_packets_processed = thr_args->T + 10;  // the number of packets processed per source will never be this much
            int source_idx_to_help = -1;
            for (int i = 0; i < numSources; i++) {
                if (source_to_num_packets_processed[i] < min_packets_processed) {
                    min_packets_processed = source_to_num_packets_processed[i];
                    source_idx_to_help = i;
                }
            }

            if (min_packets_processed == thr_args->T) {  // the most behind source has had T packets processed (or in the process of being processed) already, which means this worker thread can't help --> terminate
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
                        // pthread_exit(NULL);
                        got_a_packet = false;
                    }
                }
                lock_unlock(queue_to_help->lock, thread_idx);

                if (got_a_packet) {
                    long int packet_checksum = getFingerprint(packet->iterations, packet->seed);
                    lock_lock(queues[source_idx_to_help]->lock, thread_idx);  // conceptually we want to lock access to checksums_array[thread_idx]. We're using the queue's lock to doubly represent this

                    checksums_array[source_idx_to_help] += packet_checksum;
                    packetSignatures[source_idx_to_help][source_to_num_packets_processed[source_idx_to_help]] = packet_checksum;

                    lock_unlock(queues[source_idx_to_help]->lock, thread_idx);
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

    /* spawn n worker threads. Note in prev assignment it was n - 1 worker threads */
    int numThreads = args->numSources;
    thread_args_t thr_args[numThreads];  // must memory allocate the arg to each thread
    pthread_t thread_ids[numThreads];    // keep track of our threads

    /* Define this variable for error checking*/
    long int** packetSignatures = malloc(args->numSources * sizeof(long int*));
    for (int i = 0; i < args->numSources; i++) {
        packetSignatures[i] = malloc(args->T * sizeof(long int));
        for (int j = 0; j < args->T; j++) {
            packetSignatures[i][j] = -1;
        }
    }

    for (int i = 0; i < numThreads; i++) {
        thr_args[i].thread_idx = i;
        thr_args[i].checksums_array = checksums_array;
        thr_args[i].queues = queues;
        thr_args[i].T = args->T;
        thr_args[i].strategy = args->strategy;
        thr_args[i].numSources = args->numSources;

        thr_args[i].source_to_num_packets_processed = source_to_num_packets_processed;
        thr_args[i].packetSignatures = packetSignatures;

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

    /* Join our threads */
    for (int i = 0; i < numThreads; i++) {
        if (pthread_join(thread_ids[i], NULL) != 0) {
            printf("error joining thread!\n");
            return 1;
        }
    }

    /* Error checking: have we processed all packets from every source? */
    printf("\nsanity check: have we processed all packets from every source? \n");
    for (int i = 0; i < args->numSources; i++) {
        printf("    source_to_num_packets_processed[i]=%d\n", source_to_num_packets_processed[i]);
    }

    /*********** ERROR checking : Write to file **********/
    FILE* output_file = fopen("test_output", "a");
    if (output_file == NULL) {
        printf("Error opening output file");
        exit(1);
    }
    char buffer[MAX_STRING_LENGTH];  // used to format ints to string

    for (int i = 0; i < args->numSources; i++) {
        for (int j = 0; j < args->T; j++) {
            char source_checksum[MAX_LINE_LENGTH] = "";
            sprintf(buffer, "%ld ", packetSignatures[i][j]);  // used to format int (pos/neg) to string
            strncat(source_checksum, buffer, MAX_STRING_LENGTH);

            fputs(source_checksum, output_file);
            fputs("\n", output_file);
        }
        printf("next source\n");
    }
    fclose(output_file);
    /*********** END OF ERROR CHECKING **********/

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