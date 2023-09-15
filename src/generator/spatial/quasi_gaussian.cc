#include "quasi_gaussian.h"

#include <math.h>
#include <assert.h>
#include <vector>

// Not necessarily random, so "rnd" doesn't really fit. TODO rename?

std::vector<double> quasi_gaussian_generator::rnd_vector(size_t size,
	coordinate_gen & coord_source) const {

	std::vector<double> toRet;
	toRet.reserve(size);

	for (size_t counter = 0; counter < size; counter += 2) {
		std::pair<double, double> gaussian_sample;
		if (!center.empty() && center.size() >= 2)
			gaussian_sample = gdist.get(center[0], center[1], dispersion[0],
					qmc_sampler);
		else {
			gaussian_sample = gdist.get(dispersion[0], qmc_sampler);
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
