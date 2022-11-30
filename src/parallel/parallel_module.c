#include "parallel_module.h"

typedef struct _thread_args_t {
    int thread_idx;  // idx of the queue it should consume from + idx of where it adds checksum to
    long* checksums_array;
    queue_t** queues;
    int T;
    char strategy;
    int numSources;
    int* packets_processed_counter;
    lock_t* counter_lock;
    long int* total_checksum;
    int total_packets_goal;
} thread_args_t;

/* thr_func */
void* thr_func(void* input) {
    /* typecast arguments */
    thread_args_t* thr_args = (thread_args_t*)input;
    int thread_idx = thr_args->thread_idx;
    queue_t* thread_queue = thr_args->queues[thread_idx];
    char strategy = thr_args->strategy;
    lock_t* counter_lock = thr_args->counter_lock;
    long int* total_checksum = thr_args->total_checksum;

    /* Have worker process its queue while packets remain on them */
    int numPacketsProcessed = 0;
    long int checksumOfWorker = 0;
    while (numPacketsProcessed < thr_args->T) {
        volatile Packet_t* packet = malloc(sizeof(volatile Packet_t));

        // before dequeuing, lock the queue if the strategy requires us to. Else, don't.
        if (strategy == 'H') {
            lock_lock(thread_queue->lock, thread_idx);
            while (true) {  // while true loop here prevents dequeuing from an empty queue
                if (dequeue(thread_queue, packet) == SUCCESS) {
                    break;
                }
            };
            lock_unlock(thread_queue->lock, thread_idx);
        }

        else if (strategy == 'L') {  // no lock needed
            while (true) {           // while true loop here prevents dequeuing from an empty queue
                if (dequeue(thread_queue, packet) == SUCCESS) {
                    break;
                }
            };
        }
        long int packet_checksum = getFingerprint(packet->iterations, packet->seed);
        checksumOfWorker += packet_checksum;

        lock_lock(counter_lock, thread_idx);
        *total_checksum += packet_checksum;
        lock_unlock(counter_lock, thread_idx);

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
    int numSources = thr_args->numSources;
    int* packets_processed_counter = thr_args->packets_processed_counter;
    lock_t* counter_lock = thr_args->counter_lock;
    long int* total_checksum = thr_args->total_checksum;
    int total_packets_goal = thr_args->total_packets_goal;

    /* Define other variables to improve quality of life coding */
    int home_idx = thread_idx;

    /* Our worker keeps working until all packets from all queues have been processed */
    while (true) {
        volatile Packet_t* packet = malloc(sizeof(volatile Packet_t));

        /* Check: Can this worker's home queue be worked on? */
        if (!queue_is_empty(queues[home_idx])) {
            bool got_a_packet = false;  // did this thread get a packet to work on?

            lock_lock(queues[home_idx]->lock, thread_idx);
            if (!queue_is_empty(queues[home_idx])) {
                got_a_packet = true;
                if (dequeue(queues[home_idx], packet) == FAILURE) {
                    printf("you should never see this 1 \n");  // if we've locked the queue and then checked that it's non-empty,
                    got_a_packet = false;
                }
            }
            lock_unlock(queues[home_idx]->lock, thread_idx);

            if (got_a_packet) {
                long int packet_checksum = getFingerprint(packet->iterations, packet->seed);

                lock_lock(counter_lock, thread_idx);  // conceptually we want to lock access to checksums_array[thread_idx]. We're using the queue's lock to doubly represent this
                *packets_processed_counter += 1;
                *total_checksum += packet_checksum;
                lock_unlock(counter_lock, thread_idx);
            }
        }

        /* Find another queue to help out */
        else {
            /* Loop through to find a non-empty queue to help out */
            int queue_idx_to_help_out = -1;
            for (int i = 0; i < numSources; i++) {
                if (!queue_is_empty(queues[i])) {
                    queue_idx_to_help_out = i;
                    queue_t* queue_to_help = queues[queue_idx_to_help_out];
                    bool got_a_packet = false;

                    lock_lock(queue_to_help->lock, thread_idx);
                    if (!queue_is_empty(queue_to_help)) {
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

                        lock_lock(counter_lock, thread_idx);  // conceptually we want to lock access to checksums_array[thread_idx]. We're using the queue's lock to doubly represent this
                        *packets_processed_counter += 1;
                        *total_checksum += packet_checksum;
                        lock_unlock(counter_lock, thread_idx);
                    }
                }
            }
        }

        if (*packets_processed_counter == total_packets_goal) {
            pthread_exit(NULL);
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

    /* Create numSources many queues + locks associated with those queues */
    queue_t* queues[args->numSources];
    for (int i = 0; i < args->numSources; i++) {
        queues[i] = create_queue();
        queues[i]->lock = malloc(sizeof(lock_t));  // reserve memory for this queue's lock
        if (args->lock_type == '3') {
            queues[i]->lock->locktype = TAS_LOCK_TYPE;
        } else if (args->lock_type == '4') {
            queues[i]->lock->locktype = TTAS_LOCK_TYPE;
        }
        lock_init(queues[i]->lock);  // initialize this queue's lock
    }

    /* Create our global counter + a lock for it */
    int packets_processed_counter = 0;
    int total_packets_goal = args->numSources * args->T;

    lock_t* counter_lock = malloc(sizeof(lock_t));
    if (args->lock_type == '3') {
        counter_lock->locktype = TAS_LOCK_TYPE;
    } else if (args->lock_type == '4') {
        counter_lock->locktype = TTAS_LOCK_TYPE;
    }
    lock_init(counter_lock);

    /* Define our total checksum */
    long int total_checksum = 0;

    /* Start our timer */
    StopWatch_t* stopwatch = malloc(sizeof(StopWatch_t));
    startTimer(stopwatch);

    /* spawn n worker threads. Note in prev assignment it was n - 1 worker threads */
    int numThreads = args->numSources;
    thread_args_t thr_args[numThreads];  // must memory allocate the arg to each thread
    pthread_t thread_ids[numThreads];    // keep track of our threads

    /* Spawn our threads */
    for (int i = 0; i < numThreads; i++) {
        thr_args[i].thread_idx = i;
        thr_args[i].checksums_array = checksums_array;
        thr_args[i].queues = queues;
        thr_args[i].T = args->T;
        thr_args[i].strategy = args->strategy;
        thr_args[i].numSources = args->numSources;
        thr_args[i].packets_processed_counter = &packets_processed_counter;
        thr_args[i].counter_lock = counter_lock;
        thr_args[i].total_checksum = &total_checksum;
        thr_args[i].total_packets_goal = total_packets_goal;

        // Our threads run different functions depending on the strategy
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

    // Have dispatcher go through and put packets onto the queues
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

    /* Free memory for the queues and locks we used */
    for (int i = 0; i < args->numSources; i++) {
        lock_destroy(queues[i]->lock);
        free(queues[i]);
    }

    stopTimer(stopwatch);
    double elapsed_time = getElapsedTime(stopwatch);
    printf("total checksum=%ld\n", total_checksum);
    printf("elapsed_time: %f\n", elapsed_time);

    /* Print total checksum for correctness */

    free(stopwatch);

    return EXIT_SUCCESS;
}