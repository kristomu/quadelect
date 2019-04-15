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

// Given a scenario with a not-full Smith set, output a reduced scenario
// with a full Smith set (and with pairwise victories between members of
// the set being the same as in the input), as well as a relabeling of
// which candidates in the input correspond to which candidates in the
// output.
scenario_reduction isda_reduction::get_ISDA_reduction(
	const copeland_scenario & in) const {

	scenario_reduction out;

	// 1. Get the Smith set size of in. If it's equal to numcands, just
	// return in unperturbed.

	std::vector<std::vector<bool> > copeland_matrix =
		in.get_copeland_matrix();

	std::vector<bool> smith_cands = smith_set(copeland_matrix);

	if (smith_set_size(smith_cands) == in.get_numcands()) {
		out.to_scenario = in;
		out.cand_relabeling.resize(in.get_numcands());
		std::iota(out.cand_relabeling.begin(), out.cand_relabeling.end(), 0);

		return out;
	}

	// 2. Get the relabeling (not quite a permutation).
	// NOTE: Remove this as it's not what we need.
	size_t i, j;
	for (i = 0; i < copeland_matrix.size(); ++i) {
		if (!smith_cands[i]) continue;
		out.cand_relabeling.push_back(i);
	}

	// 2. Create a new Copeland matrix of size equal to the Smith set,
	// and copy over from the Copeland matrix, ignoring rows and columns
	// corresponding to the Smith losers.

	std::vector<int> smith_cand_idx;
	for (i = 0; i < in.get_numcands(); ++i) {
		if (smith_cands[i]) { smith_cand_idx.push_back(i); }
	}

	std::vector<std::vector<bool> > new_copeland_matrix;

	for (i = 0; i < smith_cand_idx.size(); ++i) {
		std::vector<bool> new_row;

		for (j = 0; j < smith_cand_idx.size(); ++j) {
			new_row.push_back(copeland_matrix[smith_cand_idx[i]]
				[smith_cand_idx[j]]);
		}
		new_copeland_matrix.push_back(new_row);
	}

	out.to_scenario = copeland_scenario(new_copeland_matrix);

	return out;
}

bool isda_reduction::set_scenario_constraints(copeland_scenario before,
	copeland_scenario after, const std::string before_name,
	const std::string after_name) {

	interposing_constraints = constraint_set();

	// If before and after scenarios have full Smith sets, then nothing
	// needs to be done, because the ordinary relative criterion handles
	// that scenario. We should return true so the original relative
	// criterion check is accepted in test_generator.
	if (smith_set_size(before) == before.get_numcands() &&
		smith_set_size(after) == after.get_numcands()) {
		inner_before = before;
		inner_after = after;
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
		swap(alters_before, alters_after);
		return true;
	}

	// If A is eliminated, then we can't do the reduction, since it will
	// remove the A candidate.
	if (!smith_set(after)[0]) {
		return false;
	}

	// Get the destination set and candidate relabeling.
	// order = {3, 2, 0, 1} means
	// what will be the first candidate (A) in the output is D (#3) in the
	// input.
	//std::cout << std::endl;
	scenario_reduction destination = get_ISDA_reduction(after);
	/*std::cout << "ISDA reduction info: cand_relabeling: ";
	std::copy(destination.cand_relabeling.begin(),
		destination.cand_relabeling.end(), std::ostream_iterator<size_t>(cout, " "));
	std::cout << std::endl;
	std::cout << "ISDA reduction info: After Smith set: ";
	for (bool included: smith_set(after)) {
		if (included) { std::cout << "T"; }
		else { std::cout << "F"; }
	}
	std::cout << std::endl << "after: " << destination.to_scenario.to_string() << std::endl;
	std::cout << std::endl;*/

	// Get the destination Smith set, and an elimination constraint that
	// links these together.
	elimination_spec.clear();
	int i = 0;
	for (bool included: smith_set(after)) {
		if (included) {
			elimination_spec.push_back(i++);
		} else {
			elimination_spec.push_back(-1);
		}
	}
	/*std::cout << "Elimination spec: ";
	std::copy(elimination_spec.begin(), elimination_spec.end(),
		std::ostream_iterator<int>(cout, " "));
	std::cout << std::endl;*/
	elimination_util_const euc(elimination_spec);

	interposing_constraints.add(euc.relative_constraints(
		before_name, after_name));
	/*interposing_constraints.add(general_const::all_nonnegative(
		interposing_constraints));*/

	// TODO: Handle the situation where the after scenario is not
	// canonical. Has to be done above and also change who the B
	// candidate becomes after.
	inner_before = before;
	inner_after = destination.to_scenario;

	std::cout << "(ISDA) ";

	/*std::cout << "Debug ISDA reduction: " << before.to_string() << " -> " <<
		after.to_string() <<  " became: " << inner_before.to_string()
		<< " -> "<< inner_after.to_string() << " ";*/
	alters_before = false;
	alters_after = true;

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