// Gaussian generator. Ooh, how I love inheritance and polymorphism.

// Serious revamping will be needed to do Yee properly here.

#include "spatial.h"
#include "gaussian.h"

#include <math.h>
#include <assert.h>
#include <vector>

// Not necessarily random, so "rnd" doesn't really fit. TODO rename?

// TODO: This pulls two dimensions at a time even with a higher dimension
// count, which makes quasirandom work badly with num dimensions != 2.
// FIX by not ending query until we're done sampling the whole vector.
// Also find some way to deal with odd dimensions. Use the inversion method
// for the last coordinare?
std::vector<double> gaussian_generator::rnd_vector(size_t size,
	coordinate_gen & coord_source) const {

	std::vector<double> toRet;
	toRet.reserve(size);

	for (size_t counter = 0; counter < size; counter += 2) {
		std::pair<double, double> gaussian_sample;
		if (!center.empty() && center.size() >= 2)
			gaussian_sample = gdist.get_2D(center[0], center[1], dispersion[0],
					coord_source);
		else {
			gaussian_sample = gdist.get_2D(dispersion[0], coord_source);
		}

		toRet.push_back(gaussian_sample.first);
		toRet.push_back(gaussian_sample.second);
	}

	while (toRet.size() > size) {
		toRet.pop_back();
	}

	return (toRet);
}

std::string gaussian_generator::name() const {

	std::string stub = "Gaussian, ";

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
