import subprocess
import constants


# parallel execution time
print("----parallel execution times----")
awesome_times = []
HomeQueue_times = []
for (T, W) in [(10000, 1000), (10000, 2000), (7000, 3000), (7000, 4000), (7000, 5000), (4000, 6000), (4000, 7000), (3000, 8000)]:
    print(f"for W={W}")

    n = 8

    trial_num = 0

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

    awesome_times.append(time_awesome)
    HomeQueue_times.append(time_HomeQueue)

print(awesome_times)
print(HomeQueue_times)




'''
My strategy will outperform in the case with high W for Exponential
'''