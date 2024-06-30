import subprocess, time
import numpy as np

elapsed_times = []
i=1

while True:
	start = time.time()
	subprocess.run("../../../yee_reference_benchmark",
		stdout=subprocess.DEVNULL)
	elapsed = time.time() - start

	elapsed_times.append(elapsed)
	median_time=np.median(elapsed_times)

	print("Elapsed time for iteration %d: %f seconds" % (i, elapsed))
	print("Median elapsed time so far: %f seconds" % median_time)
	i += 1