#pragma once

// I should ultimately find a way to make any spatial generator either
// random or quasi-random. The problem is that the low-discrepancy
// sequences don't project well, so we have to know the number of
// dimensions of the space ahead of time. On the other hand, for actual
// random numbers, we just pull numbers one at a time.

// The right solution will probably involve a "pull x numbers" function
// shared by low discrepancy generators and RNGs, where if x != specified
// dimension, the quasi RNG just throws an exception, whereas an RNG doesn't
// mind.

// But for now, witness this very inelegant retrofit. It's also forced to 2D
// due to details involving the conversion process from uniform to Gaussian;
// TODO fix later.

// Even though spatial generators now take *any* coordinate generator, we
// still need a mutable QMC object, because we have no way of making
// ballot generators accept nonrandom sources. This has to do with some
// generators requiring random sources or resampling, or being very slow
// without - e.g. impartial culture could be done with drand(), but the
// double generation would be much slower than generating an integer...
// So the redesign is not done yet.

#include "spatial.h"
#include "../../stats/quasirandom/r_sequence.h"
#include "../../stats/distributions/gaussian.h"

#include <vector>
#include <list>

class quasi_gaussian_generator : public spatial_generator {
	private:
		mutable r_sequence qmc_sampler;
		gaussian_dist gdist;

	protected:
		std::vector<double> rnd_vector(size_t size,
			coordinate_gen & coord_source) const;

	public:
		quasi_gaussian_generator() : spatial_generator(), qmc_sampler(2) {
			uses_center = true;
			uses_dispersion = true; set_dispersion(0.2);
		}
		quasi_gaussian_generator(bool compress_in) : spatial_generator(
				compress_in), qmc_sampler(2) {
			uses_center = true;
			uses_dispersion = true; set_dispersion(0.2);
		}
		quasi_gaussian_generator(bool compress_in, bool do_truncate)
			: spatial_generator(compress_in, do_truncate), qmc_sampler(2) {
			uses_center = true; uses_dispersion = true;
			set_dispersion(0.2);
		}

		std::string name() const;
};