#include "types.h"

bool tas_lock_init(tas_lock_t* tas_lock);

bool tas_lock_lock(tas_lock_t* tas_lock);

bool tas_lock_unlock(tas_lock_t* tas_lock);

bool tas_lock_destroy(tas_lock_t* tas_lock);