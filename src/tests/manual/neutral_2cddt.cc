#include "neutral_2cddt.h"

two_neutrality_test::two_neutrality_test() {

	two_way_tie = parser.interpret_ballots({
		"1: A>B",
		"1: B>A"}, false).second;
}

bool two_neutrality_test::passes(election_method & method) const {

	ordering outcome = method.elect(two_way_tie, 2, true);

	// We pass if the last ranked candidate has the
	// same score as the first ranked, i.e. they're tied.

	double first_score = outcome.begin()->get_score();
	double last_score = outcome.rbegin()->get_score();

	return first_score == last_score;
}