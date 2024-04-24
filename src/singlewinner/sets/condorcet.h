// This set method produces the "Condorcet set", which is the CWs ranked in
// order if there is a CW, otherwise every member is equally ranked.

// Now also the "Condorcet-nonloser set", which ranks all candidates who
// aren't Condorcet losers over the Condorcet loser, if there is one.

#pragma once

#include "../method.h"
#include "../pairwise/method.h"

class condorcet_or_loser : public pairwise_method {

	private:
		bool winner;

	public:
		// Returns -1 if none, otherwise the extreme candidate
		// (winner or loser).
		int get_extreme_candidate(const abstract_condmat & input,
			bool get_winner, const std::vector<bool> & hopefuls) const;

		int get_CW(const abstract_condmat & input,
			const std::vector<bool> & hopefuls) const {

			return get_extreme_candidate(input, true, hopefuls);
		}

		std::pair<ordering, bool> pair_elect(const abstract_condmat & input,
			const std::vector<bool> & hopefuls,
			cache_map * cache, bool winner_only) const;

		condorcet_or_loser(bool winner_in) : pairwise_method(CM_WV) {
			winner = winner_in;
			type_matters = false; update_name();
		}

		std::string pw_name() const {
			if (winner) {
				return "Condorcet";
			} else {
				return "Condorcet non-loser";
			}
		}

};

class condorcet_set : public condorcet_or_loser {

	public:
		condorcet_set() : condorcet_or_loser(true) {
		}
};

class condorcet_nonloser_set : public condorcet_or_loser {

	public:
		condorcet_nonloser_set() : condorcet_or_loser(false) {
		}
};