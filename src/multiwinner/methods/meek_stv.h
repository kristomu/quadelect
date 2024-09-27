#pragma once

#include "singlewinner/positional/simple_methods.h"

#include "methods.h"
#include <vector>
#include <list>

// TODO: Rational numbers. Nobody's done it before, so WTH not?
// Also TODO, test, because I'm getting really bad performance here.
// Maybe add BTR-STV and STV-ME to this?

// Perhaps change its name from Meek STV to "Computerized STV" or similar, since
// we also have Warren in here at this point.

// This is sufficiently different from ordinary STV that it merits its own
// class.

class MeekSTV : public multiwinner_method {
	private:
		void absorb_ballot_meek(const ballot_group & ballot,
			const std::vector<double> & weighting,
			std::vector<double> & candidate_tally, double & excess) const;
		void absorb_ballot_warren(const ballot_group & ballot,
			const std::vector<double> & weighting,
			std::vector<double> & candidate_tally, double & excess) const;
		double absolute_quota_error(const std::vector<double> & cand_tally,
			const council_t & elected, double quota) const;

		bool warren;

	public:
		council_t get_council(size_t council_size,
			size_t num_candidates, const election_t & ballots) const;

		MeekSTV(bool use_warren) {
			warren = use_warren;
		}

		std::string name() const {
			if (warren) {
				return ("Warren STV");
			} else {
				return ("Meek STV");
			}
		}
};