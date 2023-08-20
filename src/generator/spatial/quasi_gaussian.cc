#include "quasi_gaussian.h"

#include <math.h>
#include <assert.h>
#include <vector>

std::pair<double, double> quasi_gaussian_generator::grnd(double sigma_in,
	rng & random_source) const {

	double x = 0, y = 0, rad = 0;

	std::vector<double> quasi_out;

	// Use the Box-Muller transform because I'm not sure how rejection
	// sampling will interact with the low disc sequence. It may be fine.
	// TODO, test.

	// At least Box-Muller itself is fine:
	// ÖKTEN, Giray; GÖNCÜ, Ahmet. Generating low-discrepancy sequences from the
	// normal distribution: Box–Muller or inverse transform?. Mathematical and
	// Computer Modelling, 2011, 53.5-6: 1268-1281.
	// https://doi.org/10.1016/j.mcm.2010.12.011

	quasi_out = qmc_sampler.next();

	double r = sqrt(-2 * log(quasi_out[0]));
	double theta = 2 * M_PI * quasi_out[1];

	return std::pair<double, double>(
			sigma_in * r * cos(theta),
			sigma_in * r * sin(theta));

	// Okay, now we have (x,y) within the unit circle. Transform to get a
	// random Gaussian distributed variable.
	double conv_factor = sigma_in * sqrt(-2.0 * log(rad) / rad);
	return (std::pair<double, double>(x * conv_factor, y * conv_factor));
}

std::pair<double, double> quasi_gaussian_generator::grnd(double xmean,
	double ymean,
	double sigma_in, rng & random_source) const {
	std::pair<double, double> unadorned = grnd(sigma_in, random_source);

	return (std::pair<double, double>(unadorned.first + xmean,
				unadorned.second + ymean));
}

std::pair<double, double> quasi_gaussian_generator::grnd(double mean_in,
	double sigma_in, rng & random_source) const {
	return (grnd(mean_in, mean_in, sigma_in, random_source));
}

std::vector<double> quasi_gaussian_generator::rnd_vector(size_t size,
	rng & random_source) const {

	// assert size > 0 blah de blah.
	assert(size > 0);

	std::vector<double> toRet;
	toRet.reserve(size);

	for (size_t counter = 0; counter < size; counter += 2) {
		std::pair<double, double> gaussian_sample;
		if (!center.empty() && center.size() >= 2)
			gaussian_sample = grnd(center[0], center[1], dispersion[0],
					random_source);
		else {
			gaussian_sample = grnd(dispersion[0], random_source);
		}

		toRet.push_back(gaussian_sample.first);
		toRet.push_back(gaussian_sample.second);
	}

	while (toRet.size() > size) {
		toRet.pop_back();
	}

	return (toRet);
}

std::string quasi_gaussian_generator::name() const {

	std::string stub = "Quasi-Gaussian, ";

	if (!center.empty()) {
		stub += "mu = [";
		for (size_t counter = 0; counter < center.size(); ++counter) {
			stub += dtos(center[counter]);
			if (counter != center.size()-1) {
				stub += ", ";
			} else {
				stub += "], ";
			}
		}
	}

	stub += "sigma = " + dtos(dispersion[0]);

	return (stub);
}
