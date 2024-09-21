#include "bernoulli.h"
#include <vector>

std::vector<double> bernoulli_generator::rnd_vector(size_t size,
	coordinate_gen & coord_source) const {

	std::vector<double> coord = coord_source.get_coordinate(
			size);

	// Clamp.
	for (size_t i = 0; i < size; ++i) {
		if (coord[i] <= center[i]) {
			coord[i] = 1;
		} else {
			coord[i] = 0;
		}
	}

	return coord;
}

void bernoulli_generator::bias_generator(size_t num_dimensions,
	coordinate_gen & coord_source) {

	set_center(coord_source.get_coordinate(
			num_dimensions));
}