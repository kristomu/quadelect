// Should be a group of classes, even: Gaussian, linear preference, whatever.
// Maybe refactor with the issue proportional generator for multiwinner (into
// a generator and a verifier)

// BLUESKY: Use Gaussian integrals to find an exact solution (requires
// knowledge of the areas of voters that vote A > B > C, etc)

#pragma once

#include "stats/distributions/gaussian.h"

#include "spatial.h"
#include <vector>
#include <list>


class gaussian_generator : public spatial_generator {
	private:
		gaussian_dist gdist;

		// Hack
	public:
		std::vector<double> rnd_vector(size_t size,
			coordinate_gen & coord_source) const;

		//public:
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

		double get_mean_utility(
			coordinate_gen & coord_source, size_t num_voters,
			size_t numcands, size_t iterations) const;

		// Should this be here or should it be extracted into
		// gaussian_dist? I don't know, let's keep it here for now.
		// Calculate the pdf for the given point.

		// I could also make this static, or an isolated function.

		double pdf(const std::vector<double> & point) const;

		std::string name() const;
};