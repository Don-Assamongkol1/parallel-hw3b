import subprocess
import constants

T = 10000


# parallel execution time
print("----parallel execution times----")
for W in [1000, 2000, 4000, 8000]:
    print(f"for W={W}")

    parallel_time_awesome = []
    parallel_time_HomeQueue = []

    for n in [1, 2, 3, 7, 13, 27]:

        trial_num = 0
        mean_time_awesome = 0
        mean_time_HomeQueue = 0

        for _ in range(constants.UNIFORM_RERUN_COUNT):  # we will compute an average of the running time for dif seed vals
            rv_Awesome = subprocess.run(
                [
                    constants.PARALLEL_EXECUTABLE,
                    str(T),
                    str(n),
                    str(W),
                    constants.EXPONENTIAL_DISTRIBUTION,
                    str(trial_num),
                    '4',
                    'A'
                ],
                capture_output=True,
                text=True,
            )
            time_awesome = float(rv_Awesome.stdout.split(":")[-1].strip())
            mean_time_awesome += time_awesome

            rv_HomeQueue = subprocess.run(
                [
                    constants.PARALLEL_EXECUTABLE,
                    str(T),
                    str(n),
                    str(W),
                    constants.EXPONENTIAL_DISTRIBUTION,
                    str(trial_num),
                    '4',
                    'H'
                ],
                capture_output=True,
                text=True,
            )
            time_HomeQueue = float(rv_HomeQueue.stdout.split(":")[-1].strip())
            mean_time_HomeQueue += time_HomeQueue

            trial_num += 1

        mean_time_awesome /= constants.UNIFORM_RERUN_COUNT
        parallel_time_awesome.append(mean_time_awesome)

        mean_time_HomeQueue /= constants.UNIFORM_RERUN_COUNT
        parallel_time_HomeQueue.append(mean_time_HomeQueue)

    print(f"        parallel_times for varying n={parallel_times}")


'''
My strategy will outperform in the case with high W for Exponential
'''