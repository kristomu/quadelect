// Comma - meta-method with which to build methods like Smith,IRV.
// The meta-method first calculates the outcome of the first method (which is
// usually a set, like Smith), and then breaks the "ties" - tiers of set
// membership - according to the second method.

#include "../method.h"
#include "common/ballots.h"
#include "tools/ballot_tools.h"

#include "comma.h"

std::pair<ordering, bool> comma::elect_inner(const election_t
	& papers,
	const std::vector<bool> & hopefuls, int num_candidates, cache_map *
	cache, bool winner_only) const {

	// First get the orderings for the two base methods. Note the power of
	// cache: if we've calculated either before, that base method will
	// reduce to a simple lookup and so be very fast.

	std::pair<ordering, bool> set_result = set_method->elect_detailed(papers,
			hopefuls, num_candidates, cache, winner_only);

	std::pair<ordering, bool> spec_result = specific_method->elect_detailed(
			papers, hopefuls, num_candidates, cache, winner_only);

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

std::string comma::name() const {
	return ("[" + set_method->name() + "],[" + specific_method->name()
			+ "]");
}

