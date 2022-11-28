import constants

n = 1
for S in constants.worker_strategies:
    for L in constants.lock_types:
        for W in [25, 50, 100, 200, 400, 800]:
            # run w/ uniformly distributed packets
            # ./parallel_packet
            pass
