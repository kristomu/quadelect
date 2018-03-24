#include "bandit.h"
#include "lucb.h"
#include <vector>

#include <iostream>
#include <stdlib.h>
#include <math.h>

double LUCB::C(int num_plays, int num_plays_this, int num_bandits) const {
	// LUCB1 as mentioned in: 
	// "Best-arm Identification Algorithms for Multi-Armed
	// Bandits in the Fixed Confidence Setting", Jamieson et al.
	// (I couldn't get the best algorithm there to work.)

	double deltapart = (405.5 * num_bandits * pow(num_plays,1.1))/delta;
	double logpart = log(deltapart * log(deltapart));

	return(sqrt(logpart/(2.0*num_plays_this)));
}

double LUCB::C(int num_plays, const std::vector<Bandit> & bandits, 
	int which) const {

	return (C(num_plays, bandits[which].get_num_pulls(),
		bandits.size()));
}

std::pair<size_t, double> LUCB::get_best_bandit_so_far(int num_plays,
	const std::vector<Bandit> & bandits, size_t excluded_bandit_idx) const {

	// Select a default maxconf holder that isn't the excluded index.
	size_t maxconf_holder = 0;
	if (excluded_bandit_idx == 0) {
		maxconf_holder = 1;
	}

	double maxconf_score = bandits[maxconf_holder].get_mean() + C(num_plays,
		bandits, maxconf_holder);	

	for (size_t bandidx = 0; bandidx < bandits.size(); ++bandidx) {
		if (bandidx != maxconf_holder && bandidx != excluded_bandit_idx) {
			double candidate_score = bandits[bandidx].get_mean() + 
				C(num_plays, bandits, bandidx);

			if (candidate_score + drand48()*1e-9 > maxconf_score) {
				maxconf_score = candidate_score;
				maxconf_holder = bandidx;
			}
		}
	}

	return(std::pair<size_t, double>(maxconf_holder, maxconf_score));
}

std::pair<int, double> LUCB::get_best_bandit_so_far(
			const std::vector<Bandit> & bandits) const {

	int num_plays = 0;
	for (size_t i = 0; i < bandits.size(); ++i) {
		num_plays += bandits[i].get_num_pulls();
	}

	return (get_best_bandit_so_far(num_plays, bandits, -1));
}

// The function can be called more than once with no problem.
// If the bandits have already been pulled, it won't pull any of them
// in the init stage (1.), so that's okay.
// should be called pull_best_bandit, no?
double LUCB::pull_bandit_arms(std::vector<Bandit> & bandits, 
	int max_pulls, bool verbose) const {

	// 1. First pull every arm that hasn't been pulled at least once.
	// 2. Pull the best pair of bandits according to
	// 		the LUCB algorithm:
	//		2.1. the bandit with the best mean (h_t)
	//		2.2. the bandit with the best confidence bound, h_t 
	//				notwithstanding (t_l).
	//	3. If the confidence gap is smaller than the given tolerance,
	//			or we have exhausted the maximum number of iters,
	//			exit.

	// -- //

	double confidence_gap = INFINITY;

	//std::cout << "1." << std::endl;

	// 1.
	size_t bandidx;
	int num_plays = 0;

	// TODO: Perhaps set a flag once this is done, because it can take
	// some time just to go through when we have very large collections.

	for (bandidx = 0; bandidx < bandits.size(); ++bandidx) {
		if(bandits[bandidx].get_num_pulls() == 0) {
			bandits[bandidx].pull();
		}
		num_plays += bandits[bandidx].get_num_pulls();

		if (bandidx % 1000 == 0) {
			std::cout << "Initial pull: " << bandidx << " of " << bandits.size() << " or " << round(bandidx/(double)bandits.size() * 100 * 100)/100.0 << std::endl;
		}
	}

	//std::cout << "2. Maxiters is " << maxiters << std::endl;

	//std::cout << "2." << std::endl;

	// 2.
	for (int i = 0; i < max_pulls; i += 2) {
		std::cerr << i << "   " << max_pulls << "   \r" << std::flush;

		// 2.1. Find h_t. Break ties randomly (but not evenly; 
		//		that can be done later if required.)
		int maxmean_holder = 0;
		double maxmean_score = bandits[0].get_mean();

		for (bandidx = 1; bandidx < bandits.size(); ++bandidx) {
			if (bandits[bandidx].get_mean() + drand48()*1e-9 > 
			 maxmean_score) {
				maxmean_score = bandits[bandidx].get_mean();
				maxmean_holder = bandidx;
			}
		}

		// Play this bandit.
		bandits[maxmean_holder].pull();
		++num_plays;

		// What'd you need this for?
		if (bandits.size() == 1) {
			continue;
		}

		// 2.2. Find t_l. Same tiebreak.
		std::pair<int, double> maxconf = get_best_bandit_so_far(num_plays, 
			bandits, maxmean_holder);

		// Play this bandit.
		bandits[maxconf.first].pull();
		++num_plays;

		// 3. If the confidence gap is below tolerance, exit with 1.
		double maxmean_minus_conf = bandits[maxmean_holder].get_mean() -
			C(num_plays, bandits, maxmean_holder);

		confidence_gap = maxconf.second - maxmean_minus_conf;

		if (confidence_gap < tolerance) {
			return(1);
		}
	}

	// Adjust the confidence gap to something that's a little more linear,
	// then return it.
	// It's not quite linear, but not all that bad.
	// ??? Do I need this now?

	double adj_confidence_gap = exp(-sqrt(confidence_gap - tolerance));
	return(adj_confidence_gap);
}
