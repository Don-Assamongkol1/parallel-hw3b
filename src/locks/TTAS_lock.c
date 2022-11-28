#include "TTAS_lock.h"

bool ttas_lock_init(ttas_lock_t* ttas_lock) {
    ttas_lock->state = false;
    return true;
}

bool ttas_lock_lock(ttas_lock_t* ttas_lock) {
    while (true) {
        while (ttas_lock->state) {  // while state is true, i.e. while someone holds the lock
            ;
        }
        if (!__sync_fetch_and_or(&(ttas_lock->state), 1)) {
            return true;
        }
    }

    return true;
}

bool ttas_lock_unlock(ttas_lock_t* ttas_lock) {
    ttas_lock->state = 0;
    return true;
}

bool ttas_lock_destroy(ttas_lock_t* ttas_lock) {
    return true;
}