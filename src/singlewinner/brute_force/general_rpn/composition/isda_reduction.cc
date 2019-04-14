#include "isda_reduction.h"

#include "../../../../linear_model/constraints/relative_criteria/elimination.h"

std::vector<bool> isda_reduction::smith_set(
	const std::vector<std::vector<bool> > & copeland_matrix) const {

	std::vector<std::vector<bool> > haspath = copeland_matrix;

	int i, j, k, N = haspath.size();

	for (k = 0; k < N; ++k) {
		for (i = 0; i < N; ++i) {
			if (k == i) { continue; }
			for (j = 0; j < N; ++j) {
				if (k == j || i == j) { continue; }
				if (haspath[i][k] && haspath[k][j])
					haspath[i][j] = true;
			}
		}
	}

	std::vector<bool> in_smith(N, true);

	for (i = 0; i < N; ++i) {
		for (j = 0; j < N; ++j) {
			if (haspath[j][i] && !haspath[i][j]) {
				in_smith[i] = false;
			}
		}
	}
	return(in_smith);
}

size_t isda_reduction::smith_set_size(
	const std::vector<bool> & smith) const {

	size_t count = 0;

	for (bool x: smith) {
		if (x) {
			++count;
		}
	}

	return count;
}

size_t isda_reduction::smith_set_size(
	const copeland_scenario & scenario) const {
	return smith_set_size(smith_set(scenario.get_copeland_matrix()));
}

std::vector<bool> isda_reduction::smith_set(
	const copeland_scenario & scenario) const {
	return smith_set(scenario.get_copeland_matrix());
}

bool isda_reduction::set_scenario_constraints(copeland_scenario before,
	copeland_scenario after, const std::string before_name, 
	const std::string after_name) {

	interposing_constraints = constraint_set();

	// TODO: When true, set inner to outer and keep interposing_constraints
	// empty.

	// If before and after scenarios have full Smith sets, then nothing
	// needs to be done, because the ordinary relative criterion handles
	// that scenario. We should return true so the original relative
	// criterion check is accepted in test_generator.

	if (smith_set_size(before) == before.get_numcands() &&
		smith_set_size(after) == after.get_numcands()) {
		return true;
	}

	// If the before and after scenarios are equal and don't have full
	// Smith sets, then we don't need to test those scenarios as the
	// reduction of those scenarios will be tested elsewhere. E.g.
	// a test with before and after as a 4-candidate scenario with Smith
	// set ABC can be tested by the same test with before and after being
	// the 3-candidate scenario.

	if (before == after && smith_set_size(before) != before.get_numcands()) {
		return false;
	}

	// If neither Smith set is full, we don't need to test anything.
	// The reason is that if we fully enforce ISDA, then there will be some
	// test from a full Smith set to one of size X, and one from X candidates
	// with a full Smith set to one of size Y, and these can be composed if
	// the method passes both tests.
	if (smith_set_size(before) != before.get_numcands() &&
		smith_set_size(after) != after.get_numcands()) {
		return false;
	}

	// If the Smith set size is 1 or 2, don't proceed with creating
	// constraints: that logic is handled by the method itself and doesn't
	// need testing.
	if (smith_set_size(before) < 3 || smith_set_size(after) < 3) {
		return false;
	}

	// The base case is where the source scenario has a full Smith set
	// and the destination does not. If it's the other way around, reverse
	// the order, then reverse back after.

	if (smith_set_size(before) < smith_set_size(after)) {
		bool workable = set_scenario_constraints(after, before, after_name,
			before_name);
		if (!workable) { return false; }
		swap(inner_before, inner_after);
		return true;
	}

	// Get the destination Smith set, and an elimination constraint that
	// links these together.
	std::vector<int> elimination_spec;
	int i = 0;
	for (bool included: smith_set(after)) {
		if (included) {
			elimination_spec.push_back(i++);
		} else {
			elimination_spec.push_back(-1);
		}
	}
	elimination_util_const euc(elimination_spec);

	interposing_constraints.add(euc.relative_constraints(
		before_name, after_name));

	// TODO: Construct the after scenario
	// TODO: Handle the situation where the after scenario is not
	// canonical. Has to be done above and also change who the B
	// candidate becomes after.
	inner_before = before;

	// XXX: Magic happens here!

	return true;
}

/*bool test_generator::set_scenario_constraints(copeland_scenario before,
	copeland_scenario after, int max_numvoters,
	const relative_criterion_const & rel_criterion
	const std::string before_name, const std::string after_name) {

	// Check that the number of candidates match.
	if (before.get_numcands() != rel_criterion.get_numcands_before()) {
		return false;
	}
	if (after.get_numcands() != rel_criterion.get_numcands_after()) {
		return false;
	}

	// Construct a proper polytope for the desired scenarios and relative
	// criterion. If we get an exception, the parameters produce an
	// infeasible setup, e.g. mono-raise from a scenario with A>B to
	// a scenario with B>A is impossible since A can't lose by being
	// raised.

	constraint_set all_constraints;

	// Set constraints on the number of voters
	voter_constraints voters("v");
	all_constraints.add(voters.max_numvoters_definition(before.get_numcands(),
		before_name));
	all_constraints.add(voters.max_numvoters_definition(after.get_numcands(),
		after_name));
	all_constraints.add(voters.max_numvoters_upper_bound(max_numvoters));

	// Set scenario constraints.
	pairwise_constraints pairwise;
	all_constraints.add(pairwise.beat_constraints(before.get_short_form(),
		before_name, before.get_numcands()));
	all_constraints.add(pairwise.beat_constraints(after.get_short_form(),
		after_name, after.get_numcands()));

	// Add relative constraints linking the before- and after-election.
	all_constraints.add(rel_criterion.relative_constraints(before_name,
		after_name));

	// Set the margin of pairwise victory and add a nonnegativity
	// constraint.
	all_constraints.set_fixed_param("min_victory_margin", 0.01);
	all_constraints.add(general_const::all_nonnegative(all_constraints));

	return false;
}*/