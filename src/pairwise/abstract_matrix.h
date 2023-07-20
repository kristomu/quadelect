#ifndef _VOTE_C_AMATRIX
#define _VOTE_C_AMATRIX

// This is for the abstract Condorcet matrix base class. The way it is set up,
// it lets us derive both direct classes (condmat) and not-so-direct (cache ones
// for instance), and it might be useful in multiwinner when we get to that.

#include "../ballots.h"
#include "../tools/tools.h"

#include "types.h"

#include <iterator>
#include <iostream>


class abstract_condmat {

	protected:
		pairwise_type type;
		double num_voters;

		// Returns -1 on fail
		virtual double get_internal(size_t candidate, size_t against,
			bool raw) const = 0;

		virtual bool set_internal(size_t candidate, size_t against,
			double value) = 0;

	public:
		// If no number of voters was specified, set it to a guard
		// value, -infinity, so that failure to update will be
		// noticed.
		abstract_condmat(pairwise_type type_in) {
			type = type_in;
			num_voters = -INFINITY;
		}

		double get_magnitude(size_t candidate, size_t against) const;
		double get_magnitude(size_t candidate, size_t against,
			const std::vector<bool> & hopefuls) const;

		bool beats(size_t candidate, size_t challenger) const {
			return get_magnitude(candidate, challenger) >
				get_magnitude(challenger, candidate);
		}

		bool beats_or_ties(size_t candidate, size_t challenger) const {
			return get_magnitude(candidate, challenger) >=
				get_magnitude(challenger, candidate);
		}

		virtual double get_num_voters() const {
			return (num_voters);
		}
		void set_num_voters(double nv_in) {
			num_voters = nv_in;
		}

		virtual double get_num_candidates() const = 0;

		pairwise_type get_type() const {
			return (type);
		}

		bool set(size_t candidate, size_t against_candidate, double value);
		// Perhaps also + - / * etc

		// In case the matrix we derive is abstractly deleted.
		virtual ~abstract_condmat() {}
};

// This will get called *a lot*, so inline it.
inline double abstract_condmat::get_magnitude(size_t candidate,
	size_t against) const {
	// Make sure num_voters is properly set.
	// Move this to another function?
	assert(get_num_voters() != -INFINITY);

	return (get_internal(candidate, against, false));
}

#endif
