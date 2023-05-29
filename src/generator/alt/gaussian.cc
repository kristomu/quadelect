// Should be a group of classes, even: Gaussian, linear preference, whatever.
// Maybe refactor with the issue proportional generator for multiwinner (into
// a generator and a verifier)

// BLUESKY: Use Gaussian integrals to find an exact solution (requires
// knowledge of the areas of voters that vote A > B > C, etc)

#ifndef _BALLOT_GEN_GAUSSIAN
#define _VOTE_BG_GAUSSIAN

#include "ballots.h"
#include "uncomp/errors.cc"
#include <vector>
#include <list>

using namespace std;

class gaussian_generator : public spatial_generator {
	private:
		// TODO: Ziggurat instead.
		pair<double, double> grnd(double sigma) const;
		pair<double, double> grnd(double mean, double sigma) const;
		pair<double, double> grnd(double xmean, double ymean,
			double sigma) const;
		// Get preferences according to distance.
		// BLUESKY: Permit different norms.
		ballot_group get_dist_pref(const vector<double> & center,
			const vector<vector<double> > & cand_pos,
			double sigma) const;

		double sigma_set;
		vector<double> mean_set;

	public:
		gaussian_generator();
		gaussian_generator(bool do_truncate);
		gaussian_generator(bool do_truncate,
			const vector<double> & center, double sigma);

		list<ballot_group> get_gaussians(const vector<vector<double> > &
			cand_positions, int num_voters) const;
};

pair<double, double> gaussian_generator::grnd(double sigma) const {

	// Box-Mueller, dammit. We really should have something that requires
	// only a single invocation of the RNG, but bah.

	double x, y, rad = 0;

	while (rad > 1.0 || rad == 0) {
		// Choose x,y on the square (-1, -1) to (+1, +1)
		x = -1 + 2 * drand48();
		y = -1 + 2 * drand48();

		// Calculate the squared radius from origin to see if we're
		// within the unit circle.
		rad = x * x + y * y;
	}

	// Okay, now we have (x,y) within the unit circle. Transform to get a
	// random Gaussian distributed variable.
	double conv_factor = sigma * sqrt(-2.0 * log(rad) / rad);
	return (pair<double, double>(x * conv_factor, y * conv_factor));
}

pair<double, double> gaussian_generator::grnd(double xmean, double ymean,
	double sigma) const {
	pair<double, double> unadorned = grnd(sigma);

	return (pair<double, double>(unadorned.first + xmean,
				unadorned.second + ymean));
}

pair<double, double> gaussian_generator::grnd(double mean,
	double sigma) const {
	return (grnd(mean, mean, sigma));
}


ballot_group gaussian_generator::get_dist_pref(const vector<double> &
	center,
	const vector<vector<double> > & cand_pos, double sigma) const {

	vector<double> mypos(center.size());
	// mean is mypos
	int counter, sec;

	//cout << center[0] << " " << center[1] << endl;

	for (counter = 0; counter < center.size(); counter += 2) {
		if (counter == center.size()-1) {
			mypos[counter] = grnd(center[counter], sigma).first;
		} else {
			pair<double, double> coords = grnd(center[counter],
					center[counter+1], sigma);
			mypos[counter] = coords.first;
			mypos[counter+1] = coords.second;
		}
	}

	ballot_group to_ret(1);
	to_ret.complete = true;
	to_ret.rated = true;

	for (counter = 0; counter < cand_pos.size(); ++counter)
		to_ret.contents.insert(candscore(counter, -rmse(mypos,
					cand_pos[counter])));

	return (to_ret);
}

/*
gaussian_generator();
                gaussian_generator(bool do_truncate);
		                gaussian_generator(bool do_truncate,
						                                const vector<double> & center, double sigma);
*/
// Oops, is this indiv after all?

list<ballot_group> gaussian_generator::get_gaussians(
	const vector<vector<double> > & cand_positions,
	const vector<double> & center, int num_voters,
	double sigma) const {

	list<ballot_group> to_ret; // uncompressed

	for (int counter = 0; counter < num_voters; ++counter) {
		to_ret.push_back(get_dist_pref(center, cand_positions, sigma));
	}

	return (to_ret);
}

#endif
