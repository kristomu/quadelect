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


// Do inverse sampling. This is ever so slightly faster (5%) than
// Marsaglia's polar method.

// This code is from
// https://gist.github.com/kmpm/1211922/6b7fcd0155b23c3dc71e6f4969f2c48785371292
// and ultimately from
// WICHURA, Michael J. Algorithm AS 241: The percentage points of the normal
// distribution. Journal of the royal statistical society. Series C
// (applied statistics), 1988, 37.3: 477-484.

double qnorm(double p, double mu, double sigma) {
	if (p < 0 || p > 1) {
		throw std::invalid_argument(
			"The probality p must be bigger than 0 and smaller than 1");
	}
	if (sigma < 0) {
		throw std::invalid_argument(
			"The standard deviation sigma must be positive");
	}

	if (p == 0) {
		return std::numeric_limits<double>::infinity();
	}
	if (p == 1) {
		return std::numeric_limits<double>::infinity();
	}
	if (sigma == 0) {
		return mu;
	}

	double q, r, val;

	q = p - 0.5;

	/*-- use AS 241 --- */
	/* double ppnd16_(double *p, long *ifault)*/
	/*      ALGORITHM AS241  APPL. STATIST. (1988) VOL. 37, NO. 3
	        Produces the normal deviate Z corresponding to a given lower
	        tail area of P; Z is accurate to about 1 part in 10**16.
	*/
	if (fabs(q) <= .425) {
		/* 0.075 <= p <= 0.925 */
		r = .180625 - q * q;
		val =
			q * ((((((
									(r * 2509.0809287301226727 + 3430.575583588128105) * r +
									67265.770927008700853) * r +
								45921.953931549871457) * r + 13731.693765509461125) * r +
						1971.5909503065514427) * r + 133.14166789178437745) * r +
				3.387132872796366608)
			/ (((((((r * 5226.495278852854561 +
										28729.085735721942674) * r + 39307.89580009271061) * r +
								21213.794301586595867) * r + 5394.1960214247511077) * r +
						687.1870074920579083) * r + 42.313330701600911252) * r + 1);
	} else {
		/* closer than 0.075 from {0,1} boundary */

		/* r = min(p, 1-p) < 0.075 */
		if (q > 0) {
			r = 1 - p;
		} else {
			r = p;
		}

		r = sqrt(-log(r));
		/* r = sqrt(-log(r))  <==>  min(p, 1-p) = exp( - r^2 ) */

		if (r <= 5) {
			/* <==> min(p,1-p) >= exp(-25) ~= 1.3888e-11 */
			r += -1.6;
			val = (((((((r * 7.7454501427834140764e-4 +
											.0227238449892691845833) * r + .24178072517745061177) *
									r + 1.27045825245236838258) * r +
								3.64784832476320460504) * r + 5.7694972214606914055) *
						r + 4.6303378461565452959) * r +
					1.42343711074968357734)
				/ (((((((r *
											1.05075007164441684324e-9 + 5.475938084995344946e-4) *
										r + .0151986665636164571966) * r +
									.14810397642748007459) * r + .68976733498510000455) *
							r + 1.6763848301838038494) * r +
						2.05319162663775882187) * r + 1);
		} else {
			/* very close to 0 or 1 */
			r += -5;
			val = (((((((r * 2.01033439929228813265e-7 +
											2.71155556874348757815e-5) * r +
										.0012426609473880784386) * r + .026532189526576123093) *
								r + .29656057182850489123) * r +
							1.7848265399172913358) * r + 5.4637849111641143699) *
					r + 6.6579046435011037772)
				/ (((((((r *
											2.04426310338993978564e-15 + 1.4215117583164458887e-7) *
										r + 1.8463183175100546818e-5) * r +
									7.868691311456132591e-4) * r + .0148753612908506148525)
							* r + .13692988092273580531) * r +
						.59983220655588793769) * r + 1);
		}

		if (q < 0.0) {
			val = -val;
		}
	}

	return mu + sigma * val;
}

coord get_voter_point_inverse(
	double x_mean, double y_mean, double std_dev) {

	coord out;
	out.x = qnorm(drand48(), x_mean, std_dev);
	out.y = qnorm(drand48(), x_mean, std_dev);

	return out;
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
		const std::vector<double> * reference;

		bool operator()(cand_t a, cand_t b) const {
			return (*reference)[a] > (*reference)[b];
		}
};

sorter score_sorter;

void update_rank_vector(std::vector<cand_t> & rank_out,
	const std::vector<double> & scores, size_t numcands) {

	score_sorter.reference = &scores;

	for (size_t cand = 0; cand < numcands; ++cand) {
		rank_out[cand] = cand;
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
				voter_pt = get_voter_point_inverse(
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