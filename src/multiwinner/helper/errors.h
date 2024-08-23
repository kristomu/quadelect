#pragma once

#include <vector>

double rmse(const std::vector<double> & a, const std::vector<double> & b);

// Loosemore-Hanby, scaled by number of entries
double lhi(const std::vector<double> & a, const std::vector<double> & b);

// Sainte-Laguë Index.
double sli(const std::vector<double> & quantized, const
	std::vector<double> & pop_profile);

double webster_idx(const std::vector<double> & quantized,
	const std::vector<double> & pop_profile);

double gini(const std::vector<double> & a, const std::vector<double> & b);

double entropy(const std::vector<double> & quantized,
	const std::vector<double> & real);

double renyi_ent(const std::vector<double> & quantized,
	const std::vector<double> & real, double alpha);