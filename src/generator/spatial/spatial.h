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

#pragma once

#include "../ballotgen.h"
#include "stats/coordinate_gen.h"


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
		size_t num_dimensions; // Number of axes
		bool warren_utility;   // Use Warren's utility model?

		double distance(const std::vector<double> & a,
			const std::vector<double> & b) const;

		// If true, the candidate positions are fixed. This is used
		// for drawing Yee diagrams (since they'd be fairly useless
		// if the candidate positions were to drift around).
		bool fixed;
		std::vector<std::vector<double> > fixed_cand_positions;

	protected:
		bool uses_center, uses_dispersion;
		std::vector<double> center;
		std::vector<double> dispersion;

		virtual std::vector<double> rnd_vector(size_t size,
			coordinate_gen & coord_source) const = 0;
		virtual std::vector<double> max_dim_vector(size_t dimensions) const;

		election_t generate_ballots_int(int num_voters,
			int numcands, bool do_truncate,
			coordinate_gen & coord_source) const;

	public:
		spatial_generator() : pure_ballot_generator() {
			set_params(2, true); uses_center = false;
			uses_dispersion = false; fixed = false;
		}

		spatial_generator(bool compress_in) :
			pure_ballot_generator(compress_in) {
			set_params(2, true); uses_center = false;
			uses_dispersion = false; fixed = false;
		}

		spatial_generator(bool compress_in, bool do_truncate) :
			pure_ballot_generator(compress_in, do_truncate) {
			set_params(2, true); uses_center = false;
			uses_dispersion = false; fixed = false;
		}

		spatial_generator(bool compress_in, bool do_truncate,
			size_t num_dimensions_in, bool warren_util_in) :
			pure_ballot_generator(compress_in, do_truncate) {
			num_dimensions = 2;
			set_params(num_dimensions_in, warren_util_in);
			uses_center = false; uses_dispersion = false;
			fixed = false;
		}

		bool set_params(size_t num_dimensions_in, bool warren_util_in);

		size_t get_num_dimensions() const {
			return num_dimensions;
		}
		bool uses_warren_utility() const {
			return warren_utility;
		}

		// Returns the maximum score so that voters' ratings will be below
		// this value with probability p. Used for thresholding with
		// candidate-independent Range models.
		// Iterations gives the number of iterations to test to determine
		// the approximate value. If the subclass can determine the quantile
		// analytically, this value is ignored.
		virtual double get_score_quantile(coordinate_gen & coord_source,
			double p, size_t iterations) const;

		// These calculate E[optimal] - the expected utility of an optimal
		// candidate, and E[random] - the expected utility of a random
		// candidate. This is used to enforce a common denominator and
		// linearize VSE calculations so they can be used for bandits.

		virtual double get_optimal_utility(
			coordinate_gen & coord_source, size_t num_voters,
			size_t numcands, size_t iterations) const;
		virtual double get_mean_utility(
			coordinate_gen & coord_source, size_t num_voters,
			size_t numcands, size_t iterations) const;

		// For fixing candidate positions.
		void unfix_candidate_positions() {
			fixed = false;
		}
		bool fix_candidate_positions(int num_cands,
			const std::vector<std::vector<double> > cand_positions);
		bool fix_candidate_positions(int num_cands,
			coordinate_gen & coord_source);
		std::vector<std::vector<double> > get_fixed_candidate_pos() const;

		// For setting parameters of the associated distribution.
		// If the distribution doesn't make use of those parameters,
		// say sigma for a uniform distribution, nothing happens.

		virtual bool set_center(const std::vector<double> center_in);
		virtual bool set_dispersion(const std::vector<double> dispersion_in);

		bool set_center(double center_in) {
			return (set_center(std::vector<double>(num_dimensions,
							center_in)));
		}

		bool set_dispersion(double dispersion_in) {
			return (set_dispersion(std::vector<double>(num_dimensions,
							dispersion_in)));
		}

		std::vector<double> get_dispersion() const {
			return dispersion;
		}

		// Should also return dimensions, etc.
		virtual std::string name() const = 0;
};
