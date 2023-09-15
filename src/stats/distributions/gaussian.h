#pragma once

#include <vector>
#include "../coordinate_gen.h"

// Ripped from the Gaussian ballot generator, this currently
// only supports 2D. Fix later. TODO.

class gaussian_dist {
	private:
		std::pair<double, double> rejection_get(double sigma_in,
			coordinate_gen & coord_source) const;
		std::pair<double, double> stable_get(double sigma_in,
			coordinate_gen & coord_source) const;

	public:
		std::pair<double, double> get(double sigma_in,
			coordinate_gen & coord_source) const;
		std::pair<double, double> get(double mean_in, double sigma_in,
			coordinate_gen & coord_source) const;
		std::pair<double, double> get(double xmean, double ymean,
			double sigma_in, coordinate_gen & coord_source) const;
};