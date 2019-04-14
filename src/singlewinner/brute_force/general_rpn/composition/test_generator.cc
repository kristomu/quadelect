#include "test_generator.h"
#include "vector_ballot.h"

#include "../../../../linear_model/constraints/numvoters.h"
#include "../../../../linear_model/constraints/pairwise.h"
#include "../../../../linear_model/constraints/general.h"

std::list<ballot_group> test_generator::vector_election_to_ordering_format(
	const std::vector<double> & election, int numcands) const {

	std::vector<int> cur_cand_ordering(numcands);
	std::iota(cur_cand_ordering.begin(), cur_cand_ordering.end(), 0);

	std::list<ballot_group> out_election;
	int perm_number = 0;

	do {
		ballot_group ballot;
		ballot.complete = true;
		ballot.rated = false;
		for (int i = 0; i < numcands; ++i) {
			ballot.contents.insert(candscore(cur_cand_ordering[i],
				numcands-i));
		}
		ballot.weight = election[perm_number++];
		out_election.push_back(ballot);
	} while (std::next_permutation(cur_cand_ordering.begin(),
		cur_cand_ordering.end()));

	return out_election;
}

std::pair<constraint_set, bool> test_generator::set_scenario_constraints(
	copeland_scenario before, copeland_scenario after, int max_numvoters,
	const relative_criterion_const & rel_criterion,
	const std::string before_name, const std::string after_name) {

	std::pair<constraint_set, bool> out;
	out.second = false;

	// Check that the number of candidates match.
	if (before.get_numcands() != rel_criterion.get_numcands_before()) {
		return out;
	}
	if (after.get_numcands() != rel_criterion.get_numcands_after()) {
		return out;
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

	out.first = all_constraints;
	out.second = true;

	return out;
}

bool test_generator::set_scenarios(copeland_scenario before,
	copeland_scenario after, int max_numvoters,
	const relative_criterion_const & rel_criterion) {

	// Set up constraints. (This has been separated out to better deal
	// with composition required by ISDA later.)

	std::pair<constraint_set, bool> constraints = set_scenario_constraints(
		before, after, max_numvoters, rel_criterion, "before", "after");

	if (!constraints.second) { return false; }

	// Set up the polytope and sampler. Return true if we can do so,
	// false if we can't.
	try {
		election_polytope = constraint_polytope(constraints.first);
		sampler = billiard_sampler<constraint_polytope>(election_polytope,
			true, true, rng_seed);
	} catch (const std::runtime_error & e) {
		return false;
	}

	// Set the lookup permutations we need in order to return ballots.

	before_permutation_indices = election_polytope.
		get_all_permutations_indices(before.get_numcands(), "before");
	after_permutation_indices = election_polytope.
		get_all_permutations_indices(after.get_numcands(), "after");

	scenario_before = before;
	scenario_after = after;

	return true;
}

relative_test_instance test_generator::sample_instance(
	size_t other_candidate_idx_before, size_t other_candidate_idx_after,
	const fixed_cand_equivalences before_cand_remapping,
	const fixed_cand_equivalences after_cand_remapping) {

	Eigen::VectorXd point = sampler.billiard_walk();

	relative_test_instance out;
	size_t i;

	// Check that other_candidate_indices are correct
	// (i.e. that other_candidate_idx_before gets turned into
	// other_candidate_idx_after by the relative criterion).

	// Not available yet due to lacking access to the reference criterion
	// at this point. TODO: Fix later.

	/*if (get_before_cand_number(other_candidate_idx_after) !=
		other_candidate_idx_before) {
		throw std::runtime_error("test generator: candidate spec mismatch");
	}*/

	// This is kinda unwieldy: we go from vector<double> to
	// list<ballot_group> only so we can rotate and determine scenarios;
	// then we go right back because gen_custom_function takes vector<double>.
	// TODO at some later time: skip the middle man. Probably will involve
	// making a vector<double> election class.

	// It's now better handled to do cloning. But note that if we clone A
	// into say, A and D, then for vote-splitting, we should check that if
	// A beats B before, neither A nor D is beaten by B after. That isn't
	// implemented yet. TODO, do so once we've got plain cloning tests working.

	std::vector<double> vector_election(before_permutation_indices.size());
	for (i = 0; i < before_permutation_indices.size(); ++i) {
		vector_election[i] = point(before_permutation_indices[i]);
	}

	out.before_A.election = vector_election_to_ordering_format(
		vector_election, scenario_before.get_numcands());
	out.before_A.from_perspective_of = 0;
	out.before_A.scenario = scenario_before;

	vector_election.resize(after_permutation_indices.size());
	for (i = 0; i < after_permutation_indices.size(); ++i) {
		vector_election[i] = point(after_permutation_indices[i]);
	}

	out.after_A.election = vector_election_to_ordering_format(
		vector_election, scenario_after.get_numcands());
	out.after_A.from_perspective_of = 0;
	out.after_A.scenario = scenario_after;

	isomorphism cand_remapping = before_cand_remapping.
		get_candidate_remapping(out.before_A.scenario,
			other_candidate_idx_before);

	out.before_B.election = permute_election_candidates(out.before_A.election,
		cand_remapping.cand_permutations[0]);
	out.before_B.from_perspective_of = other_candidate_idx_before;
	out.before_B.scenario = cand_remapping.to_scenario;

	cand_remapping = after_cand_remapping.get_candidate_remapping(
		out.after_A.scenario, other_candidate_idx_after);

	out.after_B.election = permute_election_candidates(out.after_A.election,
		cand_remapping.cand_permutations[0]);
	out.after_B.from_perspective_of = other_candidate_idx_after;
	out.after_B.scenario = cand_remapping.to_scenario;

	return out;
}