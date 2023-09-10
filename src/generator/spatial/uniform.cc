#include "uniform.h"
#include <vector>


std::vector<double> uniform_generator::rnd_vector(size_t size,
	coordinate_gen & coord_source) const {

	std::vector<double> coord = coord_source.get_coordinate(
			size);

	// Rescale and offset.
	for (size_t i = 0; i < size; ++i) {
		double min = center[i] - dispersion[i],
			   max = center[i] + dispersion[i];

		coord[i] = (min + coord[i]) * (max-min);
	}

	return coord;
}