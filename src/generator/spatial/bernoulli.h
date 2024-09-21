// n-dimensional Bernoulli distribution: each coordinate is true (one)
// with probability p and false (zero) with probability (1-p). The center
// (location) variable is a vector of these p, because expected value on
// each axis is p.

#pragma once
#include "spatial.h"

class bernoulli_generator : public spatial_generator {
	protected:
		std::vector<double> rnd_vector(size_t size,
			coordinate_gen & coord_source) const;

	public:
		bernoulli_generator() : spatial_generator() {
			uses_center = true;
			uses_dispersion = false;
			set_center(0.5);
		}
		bernoulli_generator(bool compress_in) : spatial_generator(
				compress_in) {
			uses_center = true;
			uses_dispersion = false;
			set_center(0.5);
		}
		bernoulli_generator(bool compress_in, bool do_truncate)
			: spatial_generator(compress_in, do_truncate) {
			uses_center = true;
			uses_dispersion = false;
			set_center(0.5);
		}

		bernoulli_generator(bool compress_in, bool do_truncate,
			double num_dimensions_in, bool warren_util_in) :
			spatial_generator(compress_in, do_truncate,
				num_dimensions_in, warren_util_in) {
			uses_center = true; uses_dispersion = false;
			set_center(0.5);
		}

		// Sets the centers to random values between 0 and 1,
		// with the given number of dimensions. Used for binary issue
		// testing.
		void bias_generator(size_t num_dimensions,
			coordinate_gen & coord_source);

		std::string name() const {
			return ("Bernoulli");
		}
};