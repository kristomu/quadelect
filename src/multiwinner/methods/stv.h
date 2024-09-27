#pragma once

#include "singlewinner/positional/positional.h"
#include "singlewinner/positional/simple_methods.h"
#include "singlewinner/pairwise/method.h"
#include "singlewinner/pairwise/simple_methods.h"

#include "tools/ballot_tools.h"

#include "methods.h"
#include <list>

// TODO: Different quotas - either directly, or by passing a class.
// Maybe: That which is to IRNR what STV is to IRV. We really have to fix
// cardinal ratings somehow, because it naturally fits with all the positional
// methods (only the "positions" are given by the voter, so it doesn't do well
// with the positional matrix.)

// DONE: BTR-STV that uses Plurality instead of Condorcet.

// Really ought to make this more general: the loser elimination method
// doesn't matter as far as Droop proportionality is concerned. Later,
// after I've got this working.

enum btr_type { BTR_NONE = 0, BTR_COND = 1, BTR_PLUR = 2 };

class STV : public multiwinner_method {
	private:
		btr_type btr_stv;
		std::vector<bool> get_btr_stv_hopefuls(const ordering & count,
			const std::vector<bool> & uneliminated,
			size_t num_candidates, int num_elected,
			size_t council_size) const;

	public:
		council_t get_council(size_t council_size,
			size_t num_candidates, const election_t & ballots) const;

		STV(btr_type btr_stv_in) {
			if (btr_stv_in > BTR_PLUR) {
				throw std::invalid_argument("STV: Invalid BTR type specified");
			}

			btr_stv = btr_stv_in;
		}

		std::string name() const {
			switch (btr_stv) {
				case BTR_NONE: return ("STV");
				case BTR_COND: return ("STV-ME (Schulze)");
				case BTR_PLUR: return ("STV-ME (Plurality)");
				default: // going here would be a bug
					// but we can't use assert because g++ will complain
					// about non-void with no result.
					throw std::logic_error("Hardcard: Unsupported type!");
			}
		};
};