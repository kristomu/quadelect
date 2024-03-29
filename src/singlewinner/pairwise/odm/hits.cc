// The Hypertext Induced Text Search (HITS) algorithm.

#include "hits.h"


double hits::nltrans(double pairwise_value,
	double opposing_strength) const {
	return (pairwise_value * opposing_strength);
}

void hits::ir_norm(std::vector<double> & factors) const {

	double sum = 0;

	for (size_t counter = 0; counter < factors.size(); ++counter) {
		sum += factors[counter] * factors[counter];
	}

	sum = sqrt(sum);

	for (size_t counter = 0; counter < factors.size(); ++counter) {
		factors[counter] /= sum;
	}
}

double hits::get_score(double offense, double defense) const {
	return (offense); // Since they converge, o/d would be one.
}

std::string hits::odm_name() const {
	return ("HITS");
}

