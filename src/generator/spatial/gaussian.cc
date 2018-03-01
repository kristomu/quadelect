// Gaussian generator. Ooh, how I love inheritance and polymorphism.

// Serious revamping will be needed to do Yee properly here.

#include "spatial.h"
#include "gaussian.h"

#include <math.h>
#include <assert.h>
#include <vector>

using namespace std;

// TODO: FIX!!! Needs to have multidimensional sigma or none at all!

pair<double, double> gaussian_generator::grnd(double sigma_in, 
		rng & random_source) const {

	// Box-Mueller, dammit. We really should have something that requires
	// only a single invocation of the RNG in most situations, but bah.
	
	double x = 0, y = 0, rad = 0;

	while (rad > 1.0 || rad == 0) {
		// Choose x,y on the square (-1, -1) to (+1, +1)
		x = -1 + 2 * random_source.drand();
		y = -1 + 2 * random_source.drand();

		// Calculate the squared radius from origin to see if we're 
		// within the unit circle.
		rad = x * x + y * y;
	}

	// Okay, now we have (x,y) within the unit circle. Transform to get a
	// random Gaussian distributed variable.
	double conv_factor = sigma_in * sqrt(-2.0 * log(rad) / rad);
	return (pair<double, double>(x * conv_factor, y * conv_factor));
}

pair<double, double> gaussian_generator::grnd(double xmean, double ymean,
		double sigma_in, rng & random_source) const {
	pair<double, double> unadorned = grnd(sigma_in, random_source);

	return(pair<double, double>(unadorned.first + xmean,
				unadorned.second + ymean));
}

pair<double, double> gaussian_generator::grnd(double mean_in,
		double sigma_in, rng & random_source) const {
	return(grnd(mean_in, mean_in, sigma_in, random_source));
}

vector<double> gaussian_generator::rnd_vector(size_t size, 
		rng & random_source) const {

	// assert size > 0 blah de blah.
	assert(size > 0);

	vector<double> toRet;
	toRet.reserve(size);

	for (size_t counter = 0; counter < size; counter += 2) {
		pair<double, double> gaussian_sample;
		if (!center.empty() && center.size() >= 2)
			gaussian_sample = grnd(center[0], center[1], dispersion[0],
					random_source);
		else
			gaussian_sample = grnd(dispersion[0], random_source);

		toRet.push_back(gaussian_sample.first);
		toRet.push_back(gaussian_sample.second);
	}

	while (toRet.size() > size)
		toRet.pop_back();

	return(toRet);
}

string gaussian_generator::name() const {

	string stub = "Gaussian, ";

	if (!center.empty()) {
		stub += "mu = [";
		for (size_t counter = 0; counter < center.size(); ++counter) {
			stub += dtos(center[counter]);
			if (counter != center.size()-1)
				stub += ", ";
			else
				stub += "], ";
		}
	}

	stub += "sigma = " + dtos(dispersion[0]);

	return(stub);
}
