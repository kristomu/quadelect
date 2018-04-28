// N-dimensional Euclidean space. Candidates and voters are random points on
// [0...1] and rating is equal to 1 / (distance from voter to candidate). 

// TODO: Set dimensions. for now, fixed to n = 2. Also consider partial
// (correlated) dimensions, eyeing the SVD political compass, and other norms
// than L2.

// Bluesky: on a sphere? Tideman did this, I think. Or was that Hylland or 
// Stensholt?

// Or perhaps a generalization that makes use of "probability distribution"
// classes for vote and candidate position? Hm. Would be hard to keep pointers
// and such right, but oh the flexibility!

// Possibly make even more general - a "utility generator" which takes some
// function f(x,y) between cands and voters, where each voter then ranks each
// cand in order of increasing f(x,y). This is unlike, say, IC, in that there
// are utilities of some sort in the picture.

#ifndef _VOTE_BG_SPATIAL
#define _VOTE_BG_SPATIAL

#include "../ballotgen.h"

using namespace std;

// TODO: Provide "get_minimum_value" and "get_maximum_value" functions.
// Will have to do something about the Gaussian's infinite tail in that case.
// Perhaps just admit +/-INF on the Gaussian with a user parameter to set it
// to a truncated Gaussian instead, in which case the get_*_value would have
// those limits.

// TODO: make this ABC and move the uniform stuff into some "uniform_generator."
// Otherwise, names could cause confusion.

// Center is analogous to the mean, and dispersion is analogous to variance,
// but the latter may not necessarily be 1:1 with variance. For instance,
// for the uniform spatial generator, it's the furthest away we may go from
// the center on each axis.

// Dispersion might need to be a covariance matrix if we want to be fully
// general. On the other hand, I don't need that yet.

class spatial_generator : public pure_ballot_generator {
	private:
		double num_dimensions; // Number of axes
		bool warren_utility;   // Use Warren's utility model?

		double distance(const vector<double> & a, 
				const vector<double> & b) const;

		// If true, the candidate positions are fixed. This is used
		// for drawing Yee diagrams (since they'd be fairly useless
		// if the candidate positions were to drift around).
		bool fixed;
		vector<vector<double> > fixed_cand_positions;

	protected:
		bool uses_center, uses_dispersion;
		vector<double> center;
		vector<double> dispersion;

		virtual vector<double> rnd_vector(size_t size, rng &
				random_source) const = 0;
		virtual vector<double> rnd_dim_vector(double dimensions,
				rng & random_source) const;
		virtual vector<double> max_dim_vector(double dimensions) const;
		// TODO: "get random coordinate" which would be used for
		// sampling the circle of truncation in a manner consistent with
		// the points distribution.

	protected:
		list<ballot_group> generate_ballots_int(int num_voters,
				int numcands, bool do_truncate,
				rng & random_source) const;

	public:
		spatial_generator() : pure_ballot_generator() {
			set_params(2, true); uses_center = false; 
			uses_dispersion = false; }

		spatial_generator(bool compress_in) :
			pure_ballot_generator(compress_in) {
				set_params(2, true); uses_center = false;
				uses_dispersion = false; fixed = false; }

		spatial_generator(bool compress_in, bool do_truncate) :
			pure_ballot_generator(compress_in, do_truncate) {
				set_params(2, true); uses_center = false;
				uses_dispersion = false; fixed = false; }
			
		spatial_generator(bool compress_in, bool do_truncate,
				double num_dimensions_in, bool warren_util_in) :
			pure_ballot_generator(compress_in, do_truncate) {
				num_dimensions = 2;
				set_params(num_dimensions_in, warren_util_in);
				uses_center = false; uses_dispersion = false;
				fixed = false; }

		bool set_params(double num_dimensions_in, bool warren_util_in);

		double get_num_dimensions() const { return(num_dimensions); }
		bool uses_warren_utility() const { return(warren_utility); }

		// For fixing candidate positions.
		void unfix_candidate_positions() { fixed = false; }
		bool fix_candidate_positions(int num_cands, 
				const vector<vector<double> > cand_positions);
		bool fix_candidate_positions(int num_cands, rng & randomizer);
		vector<vector<double> > get_fixed_candidate_pos() const;

		// For setting parameters of the associated distribution.
		// If the distribution doesn't make use of those parameters,
		// say sigma for a uniform distribution, nothing happens.

		virtual bool set_center(const vector<double> center_in);
		virtual bool set_dispersion(const vector<double> dispersion_in);
		
		bool set_center(double center_in) {
			return(set_center(vector<double>(num_dimensions, 
				center_in)));
		}

		bool set_dispersion(double dispersion_in) {
			return(set_dispersion(vector<double>(num_dimensions, 
				dispersion_in)));
		}

		// Should also return dimensions, etc.
		virtual string name() const = 0;
};

#endif
