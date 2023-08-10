// Slash - meta-method with which to build methods like Smith/IRV.
// The meta-method first calculates the outcome of the first method (which is
// usually a set, like Smith), and then calculates the outcome of the second,
// constrained to the set of winners according to the first method. Then the
// outcome is the result of the second method, with ties broken according to
// the first.

#include "../method.h"
#include "../../ballots.h"
#include "../../tools/ballot_tools.h"

#include "slash.h"

std::pair<ordering, bool> slash::elect_inner(const std::list<ballot_group>
	& papers,
	const std::vector<bool> & hopefuls, int num_candidates, cache_map *
	cache, bool winner_only) const {

	// First get the ordering for the set method, using cache. If we have
	// cached the set result, this will be very quick.

	std::pair<ordering, bool> set_result = set_method->elect_detailed(papers,
			hopefuls, num_candidates, cache, winner_only);

	// Adjust hopefuls according to the results from the set. "As long as
	// the score is different from top rank, exclude that candidate".
	std::vector<bool> specified_hopefuls = hopefuls;
	for (ordering::const_reverse_iterator rpos = set_result.first.rbegin();
		rpos != set_result.first.rend() && rpos->get_score() !=
		set_result.first.begin()->get_score(); ++rpos) {
		specified_hopefuls[rpos->get_candidate_num()] = false;
	}

	// TODO: Check if there's only one candidate left. If so, just
	// return the set.

	// Second, get the specific method's ordering, using no cache since we
	// don't want interference with earlier methods. BLUESKY: Somehow
	// indicate (in cache) which candidates are used and which aren't, so
	// the issue disappears.

	std::pair<ordering, bool> spec_result = specific_method->elect_detailed(
			papers, specified_hopefuls, num_candidates, NULL, winner_only);

	// The rest goes as in comma: we complete one of the ballots with the
	// other. It doesn't matter which way we do it because, by definition,
	// the specified method can only have opinions about the candidates
	// that are ranked equal first by the first.

	std::pair<ordering, bool> toRet;

	// If either of the methods take a shortcut due to winner_only, the
	// result can only be counted as winner_only. But if neither does, then
	// we can count it as a complete result.
	toRet.second = set_result.second || spec_result.second;

	ordering_tools otools;
	// TODO: Fix tiebreak and use it instead.
	toRet.first = otools.ranked_tiebreak(set_result.first,
			spec_result.first, num_candidates);

	return (toRet);
}

std::string slash::name() const {
	return ("[" + set_method->name() + "]//[" + specific_method->name()
			+ "]");
}
