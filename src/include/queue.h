#include "types.h"
#include <unistd.h>

queue_t* create_queue();  // returns a malloc'd queue

int enqueue(queue_t* queue, volatile Packet_t* packet, bool includeSlowDown);

int dequeue(queue_t* queue, volatile Packet_t* packet, bool includeSlowDown);
// packet is a write-out parameter
