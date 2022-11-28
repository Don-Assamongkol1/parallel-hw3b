#include "types.h"

bool ttas_lock_init(ttas_lock_t* tas_lock);

bool ttas_lock_lock(ttas_lock_t* tas_lock);

bool ttas_lock_unlock(ttas_lock_t* tas_lock);

bool ttas_lock_destroy(ttas_lock_t* tas_lock);