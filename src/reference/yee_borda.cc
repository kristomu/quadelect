#include <algorithm>
#include <iostream>
#include <vector>
#include <cmath>

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

// The latter can be omitted with some clever preinitialization
// by observing that "a voter's ranked ballot consists all integers
// 0..numcands in some order" is an invariant once the ballot has been
// initialized once. But it would make the code considerably uglier.

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

coord get_voter_point_boxmuller(double x_mean, double y_mean,
	double std_dev) {

	coord voter_coord;

	double r = sqrt(-2 * log(drand48()));
	double theta = 2 * M_PI * drand48();

	voter_coord.x = x_mean + std_dev * r * cos(theta);
	voter_coord.y = y_mean + std_dev * r * sin(theta);

	return voter_coord;
}

// This is ever so slightly faster than Box-Muller, but it consumes
// more entropy.
coord get_voter_point_marsaglia(double x_mean, double y_mean,
	double std_dev) {

	coord voter_coord;

	// From Wikipedia, Marsaglia's polar method
	double u, v, s;
	do {
		u = drand48() * 2.0 - 1.0;
		v = drand48() * 2.0 - 1.0;
		s = u * u + v * v;
	} while (s >= 1.0 || s == 0.0);

	s = sqrt(-2.0 * log(s) / s);

	voter_coord.x = x_mean + std_dev * u * s;
	voter_coord.y = y_mean + std_dev * v * s;
	return voter_coord;
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
		const double * reference;

		bool operator()(cand_t a, cand_t b) const {
			return reference[a] > reference[b];
		}
};

sorter score_sorter;

void update_rank_vector(std::vector<cand_t> & rank_out,
	const std::vector<double> & scores, size_t numcands) {

	score_sorter.reference = scores.data();

	for (size_t i = 0; i < numcands; ++i) {
		rank_out[i] = i;
	}

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
			for (size_t b = a+1; b < numcands; ++b) {
				++condmat[voter_rank[a]][voter_rank[b]];
			}
		}
	}
}


// This would also be in a class.
std::vector<double> borda_scores;

size_t get_borda_winner(
	std::vector<std::vector<size_t> > & condmat) {

	borda_scores.resize(condmat.size());

	for (size_t incumbent = 0; incumbent < condmat.size(); ++incumbent) {
		for (size_t challenger = 0; challenger < condmat.size(); ++challenger) {
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
				voter_pt = get_voter_point_marsaglia(
						x/(double)xsize, y/(double)ysize, 0.3);
				set_voter_scores(voter_pt, candidate_coords,
					scores);
				update_rank_vector(ranked_ballots[voter],
					scores, numcands);
			}

			update_pairwise_matrix(pairwise_matrix, ranked_ballots);
			borda_winner[x][y] = get_borda_winner(pairwise_matrix);

			for (size_t i = 0; i < numcands; ++i) {
				for (size_t j = 0; j < numcands; ++j) {
					pairwise_matrix[i][j] = 0;
				}
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

	return 0;
}