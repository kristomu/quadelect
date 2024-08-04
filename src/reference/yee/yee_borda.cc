#include <algorithm>
#include <iostream>
#include <numeric>
#include <vector>
#include <cmath>

#include "../auxiliary_libs/zmg-0.90/zmgd.c"

#include "hack/msvc_random.h"

// This is a barebones implementation of Yee diagram calculations
// with Borda as the reference method. It works as follows:

// - Determine candidate coordinates: uniform on 0...1
// - For each grid point on [0..1]^2,
//		- Sample the coordinates of n voters centered at the
//			given point along a Gaussian distribution with a given
//			standard deviation
//		- For every voter:
//			- Determine the voter-candidate scores: negative distance to
//				candidate.
//			- Turn the score data into a strict ranked ballot vector
//				(permutation of 0...n)
//		- Calculate a pairwise matrix based on the ranked ballots.
//		- Run Borda using the pairwise matrix.
//		- Set the Borda winner for that x,y coordinate
//	- Do lots of times!

// Profiling shows that the hotspots are:
//	1. Condorcet matrix generation
//	2. Gaussian RV generation
//	3. Insertion sort (std::sort), and
//	4. Rank matrix clearing.

// It would seem that point 4 could be moved out of the loop, because "each
// voter lists every candidate in some order" is an invariant. But doing so
// actually slows down the execution! I have no idea why.

// It's possible that point 2 could be sped up by using a Ziggurat algorithm,
// but I don't know if it would play nice with Quasi-Monte Carlo. See
// https://github.com/eddelbuettel/rcppziggurat/

typedef size_t cand_t; // setting int here gives a 5% speedup. Worth it???

struct coord {
	double x, y;
};

// Damn, I'm possessed by the ghost of Cecil Martin!

std::vector<coord> get_candidate_points(size_t numcands) {

	std::vector<coord> candidate_coords;

	for (size_t i = 0; i < numcands; ++i) {
		coord cand_coord;
		cand_coord.x = drand48();
		cand_coord.y = drand48();
		candidate_coords.push_back(cand_coord);
	}

	return candidate_coords;
}

/* Yuck, fix later -- will probably need to refactor zmgd to use the
   standard RNG or something. */
ZMG_STATE rng_p1, rng_p2;
bool seeded = false;

// Draw a point from a bivariate normal distribution with
// the given expectation and standard deviation. Uses ZMGD for
// an ~25% speed improvement from inverse sampling.

// ZMGD is by F. R. Kschischang,
// https://www.comm.utoronto.ca/~frank/ZMG/index.html

void get_voter_point_zmgd(coord & out,
	double x_mean, double y_mean, double std_dev) {

	if (!seeded) {
		seedzmgd(&rng_p1, &rng_p2);
		seeded = true;
	}

	out.x = x_mean + zmgd(&rng_p1, &rng_p2) * std_dev;
	out.y = y_mean + zmgd(&rng_p1, &rng_p2) * std_dev;

}

double distance(const coord & a, const coord & b) {
	return sqrt((a.x-b.x)*(a.x-b.x) + (a.y-b.y) *(a.y-b.y));
}

void set_voter_scores(
	const coord & voter_coord,
	const std::vector<coord> & cand_coords,
	std::vector<double> & scores_out) {

	for (size_t i = 0; i < cand_coords.size(); ++i) {
		scores_out[i] = -distance(voter_coord, cand_coords[i]);
	}
}

// Scores to ranking code.

class sorter {
	public:
		std::vector<double>::const_iterator reference;

		bool operator()(cand_t a, cand_t b) const {
			return reference[a] > reference[b];
		}
};

sorter score_sorter;
std::vector<double> source;

void update_rank_vector(std::vector<cand_t> & rank_out,
	const std::vector<double> & scores, size_t numcands) {

	score_sorter.reference = scores.begin();

	std::iota(rank_out.begin(), rank_out.end(), 0);

	// Use the scores to sort the ranks.
	std::sort(rank_out.begin(), rank_out.end(), score_sorter);
}

// Given ranked ballots, calculate the pairwise matrix.
void update_pairwise_matrix(
	std::vector<std::vector<size_t> > & condmat,
	const std::vector<std::vector<cand_t> > & voter_ranks) {

	size_t numcands = condmat.size();

	for (const std::vector<cand_t> & voter_rank: voter_ranks) {
		for (size_t a = 0; a < numcands; ++a) {
			// Using a position iterator reduces the number of []
			// dereferences required.
			std::vector<cand_t>::iterator cond_row =
				condmat[voter_rank[a]].begin();

			for (size_t b = a+1; b < numcands; ++b) {
				++cond_row[voter_rank[b]];
			}
		}
	}
}


// This would also be in a class.
std::vector<double> borda_scores;

size_t get_borda_winner(
	std::vector<std::vector<size_t> > & condmat) {

	size_t numcands = condmat.size();

	borda_scores.resize(numcands);

	for (size_t incumbent = 0; incumbent < numcands; ++incumbent) {
		for (size_t challenger = 0; challenger < numcands; ++challenger) {
			borda_scores[incumbent] += condmat[incumbent][challenger] -
				condmat[challenger][incumbent];
		}
	}

	auto pos = std::max_element(borda_scores.begin(),
			borda_scores.end());

	return pos-borda_scores.begin();
}

std::vector<std::vector<size_t> > do_yee(
	size_t numcands, size_t numvoters, size_t xsize, size_t ysize) {
	// - Determine candidate coordinates: uniform on 0...1
// - For each grid point on [0..1]^2,
//		- Sample the coordinates of n voters centered at the
//			given point along a Gaussian distribution with a given
//			standard deviation
//		- Determine that candidate's scores: negative distance to
//			candidate.
//		- For every pair of candidates, update the pairwise matrix
//			data for that voter.
//		- Run Borda based on pairwise matrix info
//		- Add a point to the winner (stops optimization from cutting
//			this out)
//	- Do lots of times!

	std::vector<coord> candidate_coords =
		get_candidate_points(numcands);

	std::vector<std::vector<size_t> > borda_winner(
		xsize, std::vector<size_t>(ysize, 0));
	std::vector<std::vector<size_t> > pairwise_matrix(
		numcands, std::vector<size_t>(numcands, 0));
	coord voter_pt;
	std::vector<double> scores(numcands, 0);

	// This vector holds the ranked ballots. An 1D vector
	// might produce a further speedup, but then my code
	// would be even uglier.
	std::vector<std::vector<cand_t> > ranked_ballots(
		numvoters, std::vector<cand_t>(numcands));

	for (size_t x = 0; x < xsize; ++x) {
		std::cout << x << std::endl;
		for (size_t y = 0; y < ysize; ++y) {

			for (size_t voter = 0; voter < numvoters; ++voter) {
				get_voter_point_zmgd(voter_pt,
					x/(double)xsize, y/(double)ysize, 0.3);
				set_voter_scores(voter_pt, candidate_coords,
					scores);
				update_rank_vector(ranked_ballots[voter],
					scores, numcands);
			}

			update_pairwise_matrix(pairwise_matrix, ranked_ballots);
			borda_winner[x][y] = get_borda_winner(pairwise_matrix);

			for (size_t i = 0; i < numcands; ++i) {
				std::fill(pairwise_matrix[i].begin(),
					pairwise_matrix[i].end(), 0);
			}
		}
	}

	return borda_winner;
}

int main() {

	int numcands = 10; // 10
	int numvoters = 1942; // 20k
	int img_dim = 150; // 200

	std::vector<std::vector<size_t> > borda_winners =
		do_yee(numcands, numvoters, img_dim, img_dim);

	// Give a simple hash of the output. This would be expected
	// to change if you change the RNG, but not otherwise. RNGs must
	// be modified to be deterministic (i.e. same seed every time).

	unsigned int hash = 9973;

	for (const std::vector<size_t> & col: borda_winners) {
		for (size_t value: col) {
			hash = ((hash << 5) + hash) + value;
		}
	}

	std::cout << "Hash: " << hash << std::endl;

	return 0;
}