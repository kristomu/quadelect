This directory contains reference programs and benchmark standards, for seeing
how fast it's possible to do certain tasks, or as tests.

Yee diagrams - this is a benchmark standard that obtains a 4-5x speedup over quadelect, mostly due to better data structures and less abstraction overhead.

VSE - this is a test that performs Plurality VSE calculations with 99 voters, 3 candidates, and 8 dimensions on a multivariate normal with sigma=1.

multiwinner - this is an independent implementation of Sainte-Laguë index testing to figure out why the main multiwinner test program seems to reward small state-biased methods.
