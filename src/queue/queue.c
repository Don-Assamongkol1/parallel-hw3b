#include "queue.h"

queue_t* create_queue() {
    queue_t* queue = malloc(sizeof(queue_t));
    queue->head = 0;
    queue->tail = 0;

    // create an array of (packet pointers) of depth D
    queue->depth = DEPTH;
    queue->packet_array = malloc(sizeof(Packet_t*) * DEPTH);
    // the Packet_t*'s we get from getUniformPacket already point to a
    // heap-allocated (malloc'd) memory address

    lock_init(queue->lock);  // initialize this queue's lock

    return queue;
}

int enqueue(queue_t* queue, volatile Packet_t* packet) {
    if ((queue->tail - queue->head) == queue->depth) {
        return FAILURE;
    }
    queue->packet_array[queue->tail % queue->depth] = packet;
    __sync_synchronize();  // prevent memory hoisting for correctness
    queue->tail += 1;
    return SUCCESS;
}

int dequeue(queue_t* queue, volatile Packet_t* packet) {
    if ((queue->tail - queue->head) == 0) {
        return FAILURE;
    }
    *packet = *queue->packet_array[queue->head % queue->depth];
    __sync_synchronize();  // prevent memory hoisting for correctness
    queue->head += 1;
    return SUCCESS;
}
