#include "bandit.h"
#include "lilucb.h"
#include <vector>

#include <iostream>
#include <stdlib.h>
#include <math.h>

double Lil_UCB::get_sigma_sq(double minimum, double maximum) const {

	// For a distribution almost surely bounded on [b..a], (a-b)^2/4
	// suffices; see footnote 1 of the paper.
	return ((maximum-minimum)*(maximum-minimum)/4.0);
}

double Lil_UCB::C(int num_plays_this, int num_bandits,
	double sigma_sq) const {
	
	// These parameters are set according to the Heuristic version.
	double epsilon = 0, beta = 1/2.0, deltamod = delta/5.0;

	// p. 4

	double inner_log_part = log((1+epsilon) * num_plays_this/deltamod);
	double bigstuff = 2 * sigma_sq * (1 + epsilon) * log(inner_log_part) / 
		(double)num_plays_this;
	double score = (1 + beta) * (1 + sqrt(epsilon)) * sqrt(bigstuff);

	return(score);
}

double Lil_UCB::C(const std::vector<Bandit> & bandits, 
	int which, double sigma_sq) const {

	return (C(bandits[which].get_num_pulls(), bandits.size(), sigma_sq));
}

std::pair<size_t, double> Lil_UCB::get_best_bandit_so_far(
	const std::vector<Bandit> & bandits, double sigma_sq) const {

	double maxconf_score = -INFINITY;
	size_t maxconf_holder = -1;

	for (size_t bandidx = 0; bandidx < bandits.size(); ++bandidx) {
		double candidate_score = bandits[bandidx].get_mean() + 
			C(bandits, bandidx, sigma_sq);

		if (candidate_score > maxconf_score || 
			(candidate_score == maxconf_score && 
			bandits[bandidx].get_num_pulls() < 
			bandits[maxconf_holder].get_num_pulls())) {

			maxconf_score = candidate_score;
			maxconf_holder = bandidx;
		}
	}

	return(std::pair<size_t, double>(maxconf_holder, maxconf_score));
}

std::pair<size_t, double> Lil_UCB::get_best_bandit_so_far(
	const std::vector<Bandit> & bandits) const {

	double minimum = INFINITY, maximum = -INFINITY;

	for (size_t bandidx = 0; bandidx < bandits.size(); ++bandidx) {
		minimum = std::min(minimum, bandits[bandidx].get_minimum());
		maximum = std::max(maximum, bandits[bandidx].get_maximum());
	}

	double sigma_sq = get_sigma_sq(minimum, maximum);

	return(get_best_bandit_so_far(bandits, sigma_sq));
}

// The function can be called more than once with no problem.
// If the bandits have already been pulled, it won't pull any of them
// in the init stage (1.), so that's okay.
// should be called pull_best_bandit, no?
double Lil_UCB::pull_bandit_arms(std::vector<Bandit> & bandits, 
	int maxiters, bool verbose) {

	// 1. First pull every arm that hasn't been pulled at least once,
	//		and determine sigma^2.
	// 2. Pull the best bandit according to the lil'UCB algorithm
	// 3. If the exit criterion is met or we have exhausted the maximum 
	//		number of iters, exit.

	// -- //

	// 1.
	size_t bandidx;
	int total_num_pulls = 0;
	bool played_new_bandit = false;

	double minimum = INFINITY, maximum = -INFINITY;

	for (bandidx = 0; bandidx < bandits.size(); ++bandidx) {
		if(bandits[bandidx].get_num_pulls() == 0) {
			bandits[bandidx].pull();
			played_new_bandit = true;
		}
		total_num_pulls += bandits[bandidx].get_num_pulls();

		// Only show status if we've actually done some initial pulls.
		if (played_new_bandit && (bandidx % 1000 == 0)) {
			std::cout << "Initial pull: " << bandidx << " of " << bandits.size() << " or " << round(bandidx/(double)bandits.size() * 100 * 100)/100.0 << std::endl;
		}

		minimum = std::min(minimum, bandits[bandidx].get_minimum());
		maximum = std::max(maximum, bandits[bandidx].get_maximum());
	}

	double sigma_sq = get_sigma_sq(minimum, maximum);

	// Parameter also set according to spec for Heuristic.
	double lambda = 1 + 10/double(bandits.size());

	// Values are set so that if bandit size is 0, we directly return 1.
	double recordholder_pulled = INFINITY, other_pulled = 1;

	// 2.
	for (int i = 0; i < maxiters; ++i) {
		std::cerr << i << "   " << maxiters << "   \r" << std::flush;

		std::pair<int, double> maxconf = get_best_bandit_so_far(
			bandits, sigma_sq);

		// Play this bandit.
		bandits[maxconf.first].pull();
		++total_num_pulls;

		recordholder_pulled = bandits[maxconf.first].get_num_pulls();
		other_pulled = total_num_pulls - recordholder_pulled;

		if (recordholder_pulled >= 1 + lambda * other_pulled) {
			return(1);
		}
	}

	return(std::min(recordholder_pulled / (1 + lambda * other_pulled),
		1.0));
}
