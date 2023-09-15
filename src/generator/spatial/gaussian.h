// Should be a group of classes, even: Gaussian, linear preference, whatever.
// Maybe refactor with the issue proportional generator for multiwinner (into
// a generator and a verifier)

// BLUESKY: Use Gaussian integrals to find an exact solution (requires
// knowledge of the areas of voters that vote A > B > C, etc)

#pragma once

#include "../../stats/distributions/gaussian.h"

#include "spatial.h"
#include <vector>
#include <list>


class gaussian_generator : public spatial_generator {
	private:
		gaussian_dist gdist;

	protected:
		std::vector<double> rnd_vector(size_t size,
			coordinate_gen & coord_source) const;

	public:
		gaussian_generator() : spatial_generator() {
			uses_center = true;
			uses_dispersion = true; set_dispersion(0.2);
		}
		gaussian_generator(bool compress_in) : spatial_generator(
				compress_in) {
			uses_center = true;
			uses_dispersion = true; set_dispersion(0.2);
		}
		gaussian_generator(bool compress_in, bool do_truncate)
			: spatial_generator(compress_in, do_truncate) {
			uses_center = true; uses_dispersion = true;
			set_dispersion(0.2);
		}

		gaussian_generator(bool compress_in, bool do_truncate,
			double num_dimensions_in, bool warren_util_in) :
			spatial_generator(compress_in, do_truncate,
				num_dimensions_in, warren_util_in) {
			uses_center = true; uses_dispersion = true;
			set_dispersion(0.2);
		}

		std::string name() const;
};