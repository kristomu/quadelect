// A test of a quadratic version of Range STV. If it works well, it may be
// handled more formally as an abstraction of a common Range STV class.

// There may be a problem when the winning candidate doesn't have a Droop
// quota. The current hack zeroizes those vote strongly enough, but this
// means the downweight calculation isn't technically accurate. Fix later :p

// TODO: Find the source for this idea. It's an EM post somewhere, where the
// poster noted that an obvious problem generalizing STV to arbitrary points
// systems could be solved by using quadratic operators.

#pragma once

#include "methods.h"
#include <list>

class QRangeSTV : public multiwinner_method {
	private:
		std::vector<double> count_score(size_t num_candidates,
			const election_t & ballots,
			const std::list<double> & weights) const;

		ordering get_possible_winners(size_t num_candidates,
			const election_t & ballots,
			const std::list<double> & weights) const;

		// Should be put elsewhere.
		election_t normalize_ballots(
			const election_t & input) const;

		int elect_next(size_t council_size, size_t num_candidates,
			std::vector<bool> & elected,
			election_t & altered_ballots) const;

	public:
		std::list<size_t> get_council(size_t council_size, size_t num_candidates,
			const election_t & ballots) const;

		std::string name() const {
			return ("??Q-Range STV");
		}
};