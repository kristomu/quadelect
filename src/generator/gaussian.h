// Should be a group of classes, even: Gaussian, linear preference, whatever.
// Maybe refactor with the issue proportional generator for multiwinner (into
// a generator and a verifier)

// BLUESKY: Use Gaussian integrals to find an exact solution (requires
// knowledge of the areas of voters that vote A > B > C, etc)

#ifndef _VOTE_BG_GAUSSIAN
#define _VOTE_BG_GAUSSIAN

#include "spatial.h"
#include <vector>
#include <list>

using namespace std;

class gaussian_generator : public spatial_generator {
	private:
		// TODO: Ziggurat instead.
		pair<double, double> grnd(double sigma_in, 
				rng & random_source) const;
		pair<double, double> grnd(double mean_in, double sigma_in,
				rng & random_source) const;
		pair<double, double> grnd(double xmean, double ymean,
				double sigma_in, rng & random_source) const;
		// Get preferences according to distance.

	protected:
		vector<double> rnd_vector(int size, rng & random_source) const;

	public:
		// Do later.
/*		gaussian_generator();
		gaussian_generator(bool do_truncate);
		gaussian_generator(bool do_truncate, 
				const vector<double> & center, double sigma);

		list<ballot_group> get_gaussians(const vector<vector<double> > &
				cand_positions, int num_voters) const;*/

		gaussian_generator() : spatial_generator() { uses_mean = true;
			uses_sigma = true; sigma = 0.2; }
		gaussian_generator(bool compress_in) : spatial_generator(
				compress_in) { uses_mean = true; 
			uses_sigma = true; sigma = 0.2; }
		gaussian_generator(bool compress_in, bool do_truncate)
			: spatial_generator(compress_in, do_truncate) {
				uses_mean = true; uses_sigma = true; 
				sigma = 0.2; }

		gaussian_generator(bool compress_in, bool do_truncate,
				double num_dimensions_in, bool warren_util_in) :
			spatial_generator(compress_in, do_truncate,
					num_dimensions_in, warren_util_in) {
				uses_mean = true; uses_sigma = true; 
				sigma = 0.2; }

		string name() const;
};
#endif
