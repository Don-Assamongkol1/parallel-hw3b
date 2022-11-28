#include "lock.h"

void lock_init(lock_t* lock) {
    lock->lock = malloc(sizeof(__lock_t));

    if (lock->locktype == TAS_LOCK_TYPE) {
        lock->lock->tas_lock = malloc(sizeof(tas_lock_t));  // why is this needed
        tas_lock_init(lock->lock->tas_lock);
    } else if (lock->locktype == TTAS_LOCK_TYPE) {
        lock->lock->ttas_lock = malloc(sizeof(ttas_lock_t));
        ttas_lock_init(lock->lock->ttas_lock);
    }
}

void lock_lock(lock_t* lock, int threadID) {
    if (lock->locktype == TAS_LOCK_TYPE) {
        tas_lock_lock(lock->lock->tas_lock);
    } else if (lock->locktype == TTAS_LOCK_TYPE) {
        ttas_lock_lock(lock->lock->ttas_lock);
    }
}

void lock_unlock(lock_t* lock, int threadID) {
    if (lock->locktype == TAS_LOCK_TYPE) {
        tas_lock_unlock(lock->lock->tas_lock);
    } else if (lock->locktype == TTAS_LOCK_TYPE) {
        ttas_lock_unlock(lock->lock->ttas_lock);
    }
}

void lock_destroy(lock_t* lock) {
    if (lock->locktype == TAS_LOCK_TYPE) {
        tas_lock_destroy(lock->lock->tas_lock);
        free(lock->lock->tas_lock);
    } else if (lock->locktype == TTAS_LOCK_TYPE) {
        ttas_lock_destroy(lock->lock->ttas_lock);
        free(lock->lock->ttas_lock);
    }

    free(lock->lock);
    free(lock);
}