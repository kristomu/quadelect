#include "uniform.h"
#include <vector>


std::vector<double> uniform_generator::rnd_vector(size_t size,
	rng & random_source) const {

	std::vector<double> output(size, 0);

	for (size_t counter = 0; counter < size; ++counter)
		output[counter] = random_source.drand(
				center[counter]-dispersion[counter],
				center[counter]+dispersion[counter]);

	return (output);
}