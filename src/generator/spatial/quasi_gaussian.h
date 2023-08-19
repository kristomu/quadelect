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

#include "spatial.h"
#include "../../stats/quasirandom/r_sequence.h"
#include <vector>
#include <list>

class quasi_gaussian_generator : public spatial_generator {
	private:
		mutable r_sequence qmc_sampler;

		std::pair<double, double> grnd(double sigma_in,
			rng & random_source) const;
		std::pair<double, double> grnd(double mean_in, double sigma_in,
			rng & random_source) const;
		std::pair<double, double> grnd(double xmean, double ymean,
			double sigma_in, rng & random_source) const;

	protected:
		std::vector<double> rnd_vector(size_t size,
			rng & random_source) const;

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