import subprocess
import constants

T = 50000

# serial execution time
print("----serial execution times----")
for W in [1000, 2000, 4000, 8000]:
    print(f"for W={W}")

    serial_times = []
    for n in [1, 2, 3, 7]:  # [1, 2, 3, 7, 13, 27]:

        trial_num = 0

        # we will compute an average of the running time for dif seed vals
        mean_time = 0

        for _ in range(constants.EXPONENTIAL_RERUN_COUNT):
            rv = subprocess.run(
                [
                    constants.SERIAL_EXECUTABLE,
                    str(T),
                    str(n),
                    str(W),
                    constants.EXPONENTIAL_DISTRIBUTION,
                    str(trial_num),
                    # locktype_L,
                    # 'L'
                ],
                capture_output=True,
                text=True,
            )
            time = float(rv.stdout.split(":")[-1].strip())
            mean_time += time

            trial_num += 1

        mean_time /= constants.EXPONENTIAL_RERUN_COUNT
        serial_time = mean_time
        serial_times.append(mean_time)

    print(f"    serial times for varying n={serial_times}")


# parallel execution time
print("----parallel execution times----")
for W in [1000, 2000, 4000, 8000]:
    print(f"for W={W}")
    for strategy_S in ['L', 'H']:
        for locktype_L in constants.lock_types:

            print(f"    for (S,L)={(strategy_S, locktype_L)}")
            parallel_times = []
            for n in [1, 2, 3, 7]:  # [1, 2, 3, 7, 13, 27]:

                trial_num = 0
                mean_time = 0

                for _ in range(constants.EXPONENTIAL_RERUN_COUNT):  # we will compute an average of the running time for dif seed vals
                    rv = subprocess.run(
                        [
                            constants.PARALLEL_EXECUTABLE,
                            str(T),
                            str(n),
                            str(W),
                            constants.EXPONENTIAL_DISTRIBUTION,
                            str(trial_num),
                            locktype_L,
                            strategy_S
                        ],
                        capture_output=True,
                        text=True,
                    )
                    time = float(rv.stdout.split(":")[-1].strip())
                    mean_time += time

                    trial_num += 1

                mean_time /= constants.EXPONENTIAL_RERUN_COUNT
                parallel_times.append(mean_time)

            print(f"        parallel_times for varying n={parallel_times}")

            if strategy_S == 'L':
                break  # if our strategy is lock free, then we don't need to evaluate it against other lock types b/c it doesn't use locks anyway
