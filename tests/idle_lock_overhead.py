import subprocess

import constants

for lock_type in constants.lock_types:
    print(f"\n\n\nfor lock_type={lock_type}")
    for W in [25, 50, 100, 200, 400, 800]:
        # run w/ uniformly distributed packets
        # ./parallel_packet
        n = 1

        T = 10000000
        # if W >= 200: # tests are slower
            # T = 50000

        trial_num = 0

        # we will compute an average of the running time for dif seed vals
        mean_LockFreeStrategy_time = 0
        mean_HomeQueueStrategy_time = 0

        for _ in range(constants.UNIFORM_RERUN_COUNT):
            rv_HomeQueueStrategy = subprocess.run(
                [
                    constants.PARALLEL_EXECUTABLE,
                    str(T),
                    str(n),
                    str(W),
                    constants.UNIFORM_DISTRIBUTION,
                    str(trial_num),
                    lock_type,
                    'H'
                ],
                capture_output=True,
                text=True,
            )

            rv_LockFreeStrategy = subprocess.run(
                [
                    constants.PARALLEL_EXECUTABLE,
                    str(T),
                    str(n),
                    str(W),
                    constants.UNIFORM_DISTRIBUTION,
                    str(trial_num),
                    lock_type,
                    'L'
                ],
                capture_output=True,
                text=True,
            )
            LockFreeStrategy_time = float(rv_LockFreeStrategy.stdout.split(":")[-1].strip())
            mean_LockFreeStrategy_time += LockFreeStrategy_time


            HomeQueueStrategy_time = float(rv_HomeQueueStrategy.stdout.split(":")[-1].strip())
            mean_HomeQueueStrategy_time += HomeQueueStrategy_time

            trial_num += 1

        mean_LockFreeStrategy_time /= constants.UNIFORM_RERUN_COUNT
        mean_HomeQueueStrategy_time /= constants.UNIFORM_RERUN_COUNT

        print(f"    for W={W}")
        print(f"        mean_LockFreeStrategy_time={mean_LockFreeStrategy_time}")
        print(f"        mean_HomeQueueStrategy_time={mean_HomeQueueStrategy_time}")
