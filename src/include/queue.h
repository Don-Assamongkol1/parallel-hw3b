#include <unistd.h>

#include "lock.h"
#include "types.h"

queue_t* create_queue();  // returns a malloc'd queue

int enqueue(queue_t* queue, volatile Packet_t* packet);

int dequeue(queue_t* queue, volatile Packet_t* packet);  // packet is a write-out parameter

bool queue_is_empty(queue_t* queue);
