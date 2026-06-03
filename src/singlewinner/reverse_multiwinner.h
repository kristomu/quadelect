// "Reverse multiwinner" method used to check the manipulability of
// multiwinner methods when they elect n-1 winners out of n candidates.
// In such a case, a single candidate is left over and is the loser.
// Manipulators who prefer {X_2, ..., X_n} to {X_1, ..., X_(n-1)}
// rank X_n over X_1. Thus we can make a synthetic single-winner method
// that reverses the input ballots, determines the multiwinner council,
// and ranks the candidate left out first and everybody else second.
// The multiwinner n-1 out of n election on the reversed ballots is then
// manipulable iff the synthetic single-winner method is manipulable on
// the unreversed ballots.

// Note: since the multiwinner methods must break ties somehow, they
// may favor certain candidates over others. This could cause false
// positives if the distribution the election is picked from has a bias
// in favor of certain candidates. E.g. suppose that the tiebreak chooses
// {A,B}. Then due to some inconsequential change in a ballot, it later
// chooses {A,C}. This is not manipulation if the true random tiebreak
// would pick {A,B} half the time and {A,C} the other half. If the
// election distribution is neutral wrt candidate names, this kind of
// bias from forcing a deterministic tiebreaker should even out over
// time.

#pragma once

#include "singlewinner/method.h"
#include "multiwinner/methods/methods.h"

class reverse_multiwinner : public election_method {
	private:
		std::shared_ptr<multiwinner_method> base_method;

	public:
		std::pair<ordering, bool> elect_inner(
			const election_t & papers,
			const std::vector<bool> & hopefuls,
			int num_candidates, cache_map * cache,
			bool winner_only) const;

		std::string name() const {
			return "Reverse[" + base_method->name() + "]";
		}

		reverse_multiwinner(std::shared_ptr<multiwinner_method>
			base_method_in) {
			base_method = base_method_in;
		}
};
