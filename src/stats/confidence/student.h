// Given the degree of confidence desired, this class produces z-values for 
// confidence intervals on the t-distribution with n degrees of freedom, and
// can also provide the associated confidence interval.

// Algorithms taken from 
// http://stuff.mit.edu/afs/sipb.mit.edu/project/seven/build/AFNI98/mri_stats.c,
// ultimately AS 63 appl. statist. (1973), vol.22, no.3, for the incomplete 
// beta function and AS 103 for the inverse.

#ifndef __VOTE_STATS_TCI
#define __VOTE_STATS_TCI

#include <iostream>
#include <math.h>

using namespace std;

class t_confidence_int {

	private:
		double lnbeta(double p, double q) const;
		double incbeta(double x, double p, double q, double beta) const;
		double incbeta_inverse(double alpha, double p, double q, 
				double beta) const;

		double student_p2t(double pp, double dof) const;

	public:
		double z_score(double uncertainty, double dof) const;

		// Uncertainty is 1 - confidence, i.e. 0.05 for 95%.
		// Here, variance is the sample variance, not that of the 
		// sampling distribution of the sample mean.
		double interval(double mean, double variance, int num_samples,
				double uncertainty) const;
};

#endif
