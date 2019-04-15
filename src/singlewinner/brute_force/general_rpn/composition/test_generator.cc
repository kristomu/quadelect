#include "test_generator.h"
#include "vector_ballot.h"
#include "isda_reduction.h"

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

	// Add the definition of a minimum margin (required for pairwise
	// constraints).
	all_constraints.set_fixed_param("min_victory_margin", 0.01);

	// Add a nonnegativity constraint. (HACK: Done elsewhere to avoid
	// overlapping constraints.)

	out.first = all_constraints;
	out.second = true;

	return out;
}

bool test_generator::set_scenarios(copeland_scenario before,
	copeland_scenario after, int max_numvoters,
	const relative_criterion_const & rel_criterion) {

	// Handle ISDA. There are three possible situations:
	//	- Both before and after have full Smith sets, nothing needs be done.
	//	- Before has a full Smith set and after does not: ISDA comes at the
	//		end
	//	- After has a full Smith set and before does not: ISDA comes at the
	//		beginning.

	isda_reduction isda;
	if (!isda.set_scenario_constraints(before, after, "before_isda",
			"after_isda")) {
		// If it returns false, the scenario combination doesn't need to be
		// tested.
		return false;
	}

	std::string before_scenario_name = "before",
				after_scenario_name = "after";

	std::pair<constraint_set, bool> constraints;

	isda_first = false;
	isda_last = false;

	if (isda.is_interposing_required()) {

		// It shouldn't happen that both or neither of these are set.
		assert (isda.alters_before ^ isda.alters_after);

		if (isda.alters_after) {
			// before -> mono-raise -> before_isda -> after_isda
			isda_last = true;
			constraints = set_scenario_constraints(
				before, after, max_numvoters, rel_criterion, "before",
				"before_isda");
			before_scenario_name = "before";
			after_scenario_name = "after_isda";

			// TODO? Move into isda_reduction.
			for (size_t before_cddt = 0; before_cddt < before.get_numcands();
				++before_cddt) {
				if (isda.elimination_spec[before_cddt] != -1) {
					elim_permutations.push_back(std::pair<size_t, size_t>(
						before_cddt, isda.elimination_spec[before_cddt]));
				}
			}
		}

		if (isda.alters_before) {
			// before_isda -> after_isda -> mono-raise -> after
			isda_first = true;
			constraints = set_scenario_constraints(
				before, after, max_numvoters, rel_criterion, "after_isda",
				"after");
			before_scenario_name = "before_isda";
			after_scenario_name = "after";

			for (size_t after_cddt = 0; after_cddt < after.get_numcands();
				++after_cddt) {
				if (isda.elimination_spec[after_cddt] != -1) {
					elim_permutations.push_back(std::pair<size_t, size_t>(
						isda.elimination_spec[after_cddt], after_cddt));
				}
			}
		}

		before = isda.inner_before;
		after = isda.inner_after;

		/*std::cout << "DEBUG GMPL only isda constraints" << std::endl;
		isda.interposing_constraints.print_gmpl_program();
		std::cout << "All done..." << std::endl;*/

		constraints.first.add(isda.interposing_constraints);
		/*std::cout << "DEBUG GMPL" << std::endl;
		constraints.first.print_gmpl_program();
		std::cout << std::endl;*/
	} else {
		// Set up constraints without any need for ISDA.
		constraints = set_scenario_constraints(
			before, after, max_numvoters, rel_criterion,
			before_scenario_name, after_scenario_name);
	}

	if (!constraints.second) { return false; }

	constraints.first.add(general_const::all_nonnegative(constraints.first));

	// Done setting up constraints.

	numcands_before = before.get_numcands();
	numcands_after = after.get_numcands();

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
		get_all_permutations_indices(before.get_numcands(),
		before_scenario_name);
	after_permutation_indices = election_polytope.
		get_all_permutations_indices(after.get_numcands(),
		after_scenario_name);

	scenario_before = before;
	scenario_after = after;

	return true;
}

relative_test_instance test_generator::sample_instance(
	size_t other_candidate_idx_before, size_t other_candidate_idx_after,
	const fixed_cand_equivalences & before_cand_remapping,
	const fixed_cand_equivalences & after_cand_remapping) {

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

// TODO: fix access to reference criterion...
// Maybe it's better to just iterate over all triples...
bool test_generator::permitted_b_pair(size_t b_before, size_t b_after,
	const relative_criterion_const & rel_criterion) const {

	// Go through every allowed pair, and transform according to
	// relative_criterion.

	// Currently a very ugly hack is used. This should really be handled
	// when generating the pairs.

	if (!isda_first && !isda_last) {
		if (rel_criterion.get_before_cand_number(b_after) != b_before) {
			return false;
		}
		return true;
	}

	if (isda_first) {
		for (const std::pair<size_t, size_t> & permitted_pair:
			elim_permutations) {
			if (permitted_pair.first != b_before) { continue; }
			if (rel_criterion.get_before_cand_number(b_after) !=
				permitted_pair.second) { continue; }

			return true;
		}
	}

	if (isda_last) {
		for (const std::pair<size_t, size_t> & permitted_pair:
			elim_permutations) {

			if (permitted_pair.second != b_after) { continue; }
			if (rel_criterion.get_before_cand_number(permitted_pair.first) !=
				b_before) { continue; }

			return true;
		}
	}

	return false;
}