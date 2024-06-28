// Calculate the VSE for Plurality, by observing that the Plurality count
// can be rolled into the method like this, per election:

// Determine each candidate's position by drawing a d-dimensional normal.
// For i = 1...numvoters:
//		- Determine that voter's position.
//		- Calculate the Euclidean norm between the voter and each candidate.
//		- Negate to get utility (James Green-Armytage model).
//		- Increment by one the Plurality counter of the candidate with max utility.
//		- Update total mean and optimal utilities according to the utilities
//				of this voter.
//	- Let the utility of the Plurality winner be that of the candidate with max
//		Plurality counter. (Tiebreaking doesn't matter because they'll average out
//		anyway.)

// Then just repeat the process as many times as you care.

// The program both reports VSE and James Green-Armytage's chance that the optimal
// utility candidate is elected. This solves the discrepancy that had me confused.

// Callgrind benchmark results:
// (if you significantly change the hotspots, update this! XXX)

//		inclusive	self	function
//		74.53%		15.95%	rnd_normal
//		44.32%		10.01%	drand48
//		34.31%		21.44%	erand48_r
//		14.26%		 1.40%	log
//		12.87%		12,87%	__drand48_iterate
//		12.86%		12.86%	__ieee754_log_fma
//		 6.11%		 0.15%	operator delete(void *, ...)
//		 5.96%		 0.15%	operator delete(void *)

#include <algorithm>
#include <cassert>
#include <cmath>
#include <vector>
#include <iostream>
#include <iterator>

#include <time.h>

// Marsaglia's polar method, without keeping a spare.
// Preallocate destination before using!
void rnd_normal(std::vector<double> & destination,
	size_t dimensions, double sigma) {

	assert(dimensions % 2 == 0);

	double mu = 0;

	for (size_t i = 0; i < dimensions; i += 2) {

		// From Wikipedia, Marsaglia's polar method
		double u, v, s;
		do {
			u = drand48() * 2.0 - 1.0;
			v = drand48() * 2.0 - 1.0;
			s = u * u + v * v;
		} while (s >= 1.0 || s == 0.0);

		s = sqrt(-2.0 * log(s) / s);

		double z_0 = mu + sigma * u * s;
		double z_1 = mu + sigma * v * s;

		destination[i] = z_0;
		destination[i+1] = z_1;
	}
}

double sqr(double x) {
	return x*x;
}

// Returns the Euclidean norm of a-b.
double distance(const std::vector<double> & a,
	const std::vector<double> & b) {

	assert(a.size() == b.size());

	double squared_dist = 0;

	for (size_t i = 0; i < a.size(); ++i) {
		squared_dist += sqr(a[i] - b[i]);
	}

	return sqrt(squared_dist);
}

template <typename T>
std::vector<T> & operator+=(std::vector<T> & a, const std::vector<T> & b) {
	assert(a.size() == b.size());

	for (size_t i = 0; i < a.size(); ++i) {
		a[i] += b[i];
	}

	return a;
}

template <typename T, typename Q>
std::vector<T> & operator/=(std::vector<T> & a, Q divisor) {
	for (size_t i = 0; i < a.size(); ++i) {
		a[i] /= divisor;
	}

	return a;
}

double mean(std::vector<double> & a) {
	double sum = 0;

	for (size_t i = 0; i < a.size(); ++i) {
		sum += a[i];
	}

	return sum/a.size();
}

// Returns true if the optimal utility candidate was elected,
// false otherwise.

bool sim_election(
	double & total_mean_utilities,
	double & total_optimal_utilities,
	double & total_plurality_utilities,
	size_t numvoters, size_t numcands, size_t dimensions,
	double sigma) {

	// Allocate vectors we'll be using.
	std::vector<std::vector<double> > cand_positions(numcands,
		std::vector<double>(dimensions, 0));

	std::vector<double> voter_pos(dimensions, 0);

	size_t cand, voter;
	std::vector<size_t> plurality_counts(numcands, 0);
	std::vector<double> mean_election_utils(numcands, 0);
	double opt_election_utility = 0, mean_election_utility = 0;

	// Set candidate positions.
	for (cand = 0; cand < numcands; ++cand) {
		rnd_normal(cand_positions[cand], dimensions, 1);
	}

	// For each voter...
	for (voter = 0; voter < numvoters; ++voter) {

		// Set voter position and utilities, and add to the mean utilities.
		rnd_normal(voter_pos, dimensions, 1);
		std::vector<double> voter_utilities(numcands, 0);

		for (cand = 0; cand < numcands; ++cand) {
			voter_utilities[cand] = -distance(voter_pos,
					cand_positions[cand]);

			mean_election_utils[cand] += voter_utilities[cand];
		}

		// Determine the first preference candidate and increment the
		// Plurality counter.

		size_t first_pref_cand = std::max_element(voter_utilities.begin(),
				voter_utilities.end()) - voter_utilities.begin();

		plurality_counts[first_pref_cand]++;
	}

	// Normalize utilities to be mean for this election.

	mean_election_utils /= numvoters;
	mean_election_utility = mean(mean_election_utils);

	// Get the optimum and Plurality winner and increment the global
	// utility counters.

	opt_election_utility = *std::max_element(mean_election_utils.begin(),
			mean_election_utils.end());

	total_mean_utilities += mean_election_utility;
	total_optimal_utilities += opt_election_utility;

	size_t plurality_winner = std::max_element(plurality_counts.begin(),
			plurality_counts.end()) - plurality_counts.begin();

	total_plurality_utilities += mean_election_utils[plurality_winner];

	return mean_election_utils[plurality_winner] == opt_election_utility;
}

int main() {

	int numvoters = 99, numcands = 3; // E.g.
	int dimensions = 8;
	double sigma = 1;

	double total_mean_utilities = 0, total_optimal_utilities = 0,
		   total_plurality_utilities = 0;

	time_t last_displayed_info = 0;

	size_t optimal_utility_hits = 0;

	for (size_t iter = 1;; ++iter) {
		bool hit = sim_election(total_mean_utilities,
				total_optimal_utilities,
				total_plurality_utilities,
				numvoters, numcands, dimensions, sigma);

		if (hit) {
			++optimal_utility_hits;
		}

		// Note that the denominator of number of iters cancels out.
		// Thus we can use "number of iters * expectation" for all the
		// elements without running into any problems.
		double nE_chosen = total_plurality_utilities;
		double nE_random = total_mean_utilities;
		double nE_optimal = total_optimal_utilities;

		double VSE = (nE_chosen - nE_random) / (nE_optimal - nE_random);

		if ((iter & 15) == 0) {
			time_t now = time(NULL);

			if (now - last_displayed_info > 5) {
				std::cout << "Iter " << iter << ": VSE is " << VSE;
				std::cout << "\t optimal utility freq. (OUF): " <<
					optimal_utility_hits/(double)iter << std::endl;
				last_displayed_info = now;
			}
		}
	}

	return 0;
}