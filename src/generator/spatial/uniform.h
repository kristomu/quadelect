// Uniform distribution: the default is that the voter is at a random value
// on [0...1]^2.

#pragma once
#include "spatial.h"

class uniform_generator : public spatial_generator {
	protected:
		std::vector<double> rnd_vector(size_t size,
			coordinate_gen & coord_source) const;

	public:
		uniform_generator() : spatial_generator() {
			uses_center = true;
			uses_dispersion = true; set_dispersion(0.5);
			set_center(0.5);
		}
		uniform_generator(bool compress_in) : spatial_generator(
				compress_in) {
			uses_center = true;
			uses_dispersion = true; set_dispersion(0.5);
			set_center(0.5);
		}
		uniform_generator(bool compress_in, bool do_truncate)
			: spatial_generator(compress_in, do_truncate) {
			uses_center = true; uses_dispersion = true;
			set_dispersion(0.5); set_center(0.5);
		}

		uniform_generator(bool compress_in, bool do_truncate,
			double num_dimensions_in, bool warren_util_in) :
			spatial_generator(compress_in, do_truncate,
				num_dimensions_in, warren_util_in) {
			uses_center = true; uses_dispersion = true;
			set_dispersion(0.5); set_center(0.5);
		}

		std::string name() const {
			return ("Uniform spatial");
		}
};