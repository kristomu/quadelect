// This class gives confidence intervals for certain statistical
// distributions, currently t and binomial proportion confidence intervals.

// Algorithms taken from
// https://afni.nimh.nih.gov/pub/dist/src_OBSOLETE/mri_stats.c
// ultimately AS 63 appl. statist. (1973), vol.22, no.3, for the incomplete
// beta function and AS 103 for the inverse.

#ifndef __VOTE_STATS_CI
#define __VOTE_STATS_CI

#include <iostream>
#include <math.h>

using namespace std;

class confidence_int {

	private:
		double lnbeta(double p, double q) const;
		double incbeta(double x, double p, double q, double beta) const;
		double incbeta_inverse(double alpha, double p, double q,
			double beta) const;

		double qt(double p, double dof) const;
		double qbeta(double p, double a, double b) const;
	public:
		bool test() const;

		// Uncertainty is 1 - confidence, i.e. 0.05 for 95%.
		// Here, variance is the sample variance, not that of the
		// sampling distribution of the sample mean.

		// half-width gives the +/- from the mean.
		double t_interval_half_width(double uncertainty,
			double mean, double variance, int num_samples) const;

		std::pair<double, double> t_interval(double uncertainty,
			double mean, double variance, int num_samples) const;

		std::pair<double, double> bin_prop_interval(double p, int k,
			int n) const;
};

#endif
