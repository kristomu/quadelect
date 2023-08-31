#include <algorithm>
#include <iostream>
#include <vector>
#include <cmath>

// This is a barebones implementation of Yee diagram calculations
// with Borda as the reference method. It works as follows.

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

/*coord get_voter_point(double x_mean, double y_mean,
	double std_dev) {

	coord voter_coord;

	double r = sqrt(-2 * log(drand48()));
	double theta = 2 * M_PI * drand48();

	voter_coord.x = x_mean + std_dev * r * cos(theta);
	voter_coord.y = y_mean + std_dev * r * sin(theta);

	return voter_coord;
}*/

coord get_voter_point(double x_mean, double y_mean,
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

void update_pairwise_matrix(
	std::vector<std::vector<size_t> > & condmat,
	const std::vector<double> & scores) {

	size_t numcands = condmat.size();

	for (size_t incumbent = 0; incumbent < numcands; ++incumbent) {
		for (size_t challenger = 0; challenger < numcands; ++challenger) {
			if (scores[incumbent] > scores[challenger]) {
				condmat[incumbent][challenger]++;
			}
		}
	}
}

// This would be in a class
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

	for (int x = 0; x < xsize; ++x) {
		std::cout << x << std::endl;
		for (int y = 0; y < ysize; ++y) {
			for (int voter = 0; voter < numvoters; ++voter) {
				voter_pt = get_voter_point(x/(double)xsize,
						y/(double)ysize, 0.3);
				set_voter_scores(voter_pt, candidate_coords,
					scores);
				update_pairwise_matrix(pairwise_matrix, scores);
			}
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
	int numvoters = 2000; // 20k
	int img_dim = 50; // 200

	std::vector<std::vector<size_t> > borda_winners =
		do_yee(numcands, numvoters, img_dim, img_dim);

	return 0;
}