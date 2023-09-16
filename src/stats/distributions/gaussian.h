#pragma once

#include <vector>
#include "../coordinate_gen.h"

// This currently only supports 2D. Fix later. TODO.

class gaussian_dist {
	private:
		double qnorm(double p, double mu, double sigma) const;

	public:
		std::pair<double, double> get_2D(double sigma_in,
			coordinate_gen & coord_source) const;
		std::pair<double, double> get_2D(double xmean, double ymean,
			double sigma_in, coordinate_gen & coord_source) const;
};