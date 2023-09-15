#include "gaussian.h"

#include <math.h>
#include <assert.h>
#include <vector>

// If we need it later, implement a general covariance matrix.

// Normal variate generation based on rejection sampling. This works
// for independent coordinate sources, but aren't really suitable for
// non-independent ones. This could be further improved by using the
// Ziggurat algorithm.
std::pair<double, double> gaussian_dist::rejection_get(
	double sigma_in, coordinate_gen & coord_source) const {

	double x = 0, y = 0, rad = 0;

	// TODO: ??? Generalize to multiple dimensions, throwing away
	// the excess if odd. (We can't do that in stable_, however.)
	std::vector<double> coordinate(2);

	while (rad > 1.0 || rad == 0) {
		coordinate = coord_source.get_coordinate(2);
		// Choose x,y on the square (-1, -1) to (+1, +1)
		x = -1 + 2 * coordinate[0];
		y = -1 + 2 * coordinate[1];

		// Calculate the squared radius from origin to see if we're
		// within the unit circle.
		rad = x * x + y * y;
	}

	// Okay, now we have (x,y) within the unit circle. Transform to get a
	// random Gaussian distributed variable.
	double conv_factor = sigma_in * sqrt(-2.0 * log(rad) / rad);
	return (std::pair<double, double>(x * conv_factor, y * conv_factor));
}

// This always uses the same number of coordinates and so is suitable
// for Quasi-Monte Carlo.
std::pair<double, double> gaussian_dist::stable_get(
	double sigma_in, coordinate_gen & coord_source) const {

	// Use the Box-Muller transform. It should be fine for QMC:
	// ÖKTEN, Giray; GÖNCÜ, Ahmet. Generating low-discrepancy sequences from the
	// normal distribution: Box–Muller or inverse transform?. Mathematical and
	// Computer Modelling, 2011, 53.5-6: 1268-1281.
	// https://doi.org/10.1016/j.mcm.2010.12.011

	std::vector<double> coordinate = coord_source.get_coordinate(2);

	double r = sqrt(-2 * log(coordinate[0]));
	double theta = 2 * M_PI * coordinate[1];

	return std::pair<double, double>(
			sigma_in * r * cos(theta),
			sigma_in * r * sin(theta));
}

std::pair<double, double> gaussian_dist::get(
	double sigma_in, coordinate_gen & coord_source) const {

	if (coord_source.is_independent()) {
		return rejection_get(sigma_in, coord_source);
	} else {
		return stable_get(sigma_in, coord_source);
	}
}

std::pair<double, double> gaussian_dist::get(double xmean,
	double ymean, double sigma_in, coordinate_gen & coord_source) const {
	std::pair<double, double> unadorned = get(sigma_in, coord_source);

	return (std::pair<double, double>(unadorned.first + xmean,
				unadorned.second + ymean));
}