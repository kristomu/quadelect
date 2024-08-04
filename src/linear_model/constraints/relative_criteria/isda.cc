#include "isda.h"
#include "tools/cp_tools.h"

size_t isda_relative_const::calc_num_before_cands(bool elimination_first,
	std::shared_ptr<relative_criterion_const> & inner_criterion_in,
	const std::vector<int> & elimination_spec_in) const {

	// If elimination happens first, then we start with the output number of
	// candidates to the elimination constraint "criterion".

	if (elimination_first) {
		return elimination_util_const(elimination_spec_in).
			get_numcands_after();
	}

	// If elimination happens last, then we start with the inner criterion's
	// input number of candidates.

	return inner_criterion_in->get_numcands_before();
}

size_t isda_relative_const::calc_num_after_cands(bool elimination_first,
	std::shared_ptr<relative_criterion_const> & inner_criterion_in,
	const std::vector<int> & elimination_spec_in) const {

	// If elimination happens first, then we end with the inner criterion's
	// output number of candidates.

	if (elimination_first) {
		return inner_criterion_in->get_numcands_after();
	}

	// If elimination happens last, then we end with the number of
	// uneliminated candidates, i.e. the elimination constraint's number
	// of candidates after.

	return elimination_util_const(elimination_spec_in).get_numcands_after();
}

std::string isda_relative_const::name() const {
	return "ISDA(" + inner_criterion->name() + ")";
}

constraint_set isda_relative_const::smith_loser_constraints(
	std::string ballot_suffix, std::string description_suffix) const {

	// Use the standard name for the margin of victory parameter; this
	// fixed parameter is set by test_generator.
	pairwise_constraints pwconst("min_victory_margin");

	constraint_set loser_constraints;

	size_t numcands = elimination_spec.size();
	std::vector<size_t> losers, non_losers;

	for (size_t i = 0; i < elimination_spec.size(); ++i) {
		if (elimination_spec[i] == -1) {
			losers.push_back(i);
		} else {
			// Note that the pairwise constraints are linked to the
			// situation before elimination, thus we push the index,
			// not the number at that index (which would be the candidate
			// number after elimination).
			non_losers.push_back(i);
		}
	}

	for (size_t winner: non_losers) {
		for (size_t loser: losers) {
			loser_constraints.add(pwconst.beat_constraint(true,
					winner, loser, ballot_suffix, description_suffix,
					numcands));
		}
	}

	return loser_constraints;
}

constraint_set isda_relative_const::relative_constraints(
	std::string before_suffix, std::string after_suffix) const {

	constraint_set isda_linkage;

	if (isda_before) {
		// inner criterion's before ballot -> pairwise constraints ->
		// elimination by the given schedule -> before ballot
		// (fewer candidates)

		// inner criterion's before ballot -> inner criterion ->
		// after ballot (more candidates)

		// Pairwise constraints on before_suffix
		isda_linkage.add(smith_loser_constraints(
				"inner_" + before_suffix,
				"inner_" + before_suffix + "_isda_loser"));

		// elimination
		isda_linkage.add(eliminator.relative_constraints(
				"inner_" + before_suffix, before_suffix));

		// Inner criterion
		isda_linkage.add(inner_criterion->relative_constraints(
				"inner_" + before_suffix, after_suffix));
	} else {
		// before ballot -> inner criterion -> pairwise constraints ->
		// elimination by the given schedule -> after ballot.

		// Inner criterion
		isda_linkage.add(inner_criterion->relative_constraints(
				before_suffix, before_suffix + "_isda"));

		// Pairwise constraints on before_suffix + _isda go here
		isda_linkage.add(smith_loser_constraints(before_suffix + "_isda",
				before_suffix + "_isda_isda_loser"));

		// Elimination
		isda_linkage.add(eliminator.relative_constraints(
				before_suffix + "_isda", after_suffix));
	}

	return isda_linkage; // Now wasn't that easy?
}

// This is also rather ugly.

isda_relative_const::isda_relative_const(bool elimination_first,
	std::shared_ptr<relative_criterion_const> inner_criterion_in,
	std::vector<int> elimination_spec_in) :
	relative_criterion_const(
		calc_num_before_cands(elimination_first, inner_criterion_in,
			elimination_spec_in),
		calc_num_after_cands(elimination_first, inner_criterion_in,
			elimination_spec_in)
	), eliminator(elimination_spec_in) {

	isda_before = elimination_first;
	inner_criterion = inner_criterion_in;
	elimination_spec = elimination_spec_in;

	// Verify that the number of candidates prior to elimination match.
	if (isda_before) {
		assert(numcands_before == eliminator.get_numcands_after());
		assert(numcands_after == inner_criterion->get_numcands_after());

		// If the composition is impossible due to the inner criterion
		// requiring a different number of candidates than ISDA will
		// provide, throw an exception.
		if (inner_criterion_in->get_numcands_before() !=
			elimination_spec.size()) {
			throw std::runtime_error("ISDA: candidate number mismatch"
				" in criterion composition!");
		}

		// Since the input ballot is "un-eliminated" to produce a larger
		// ballot, and this is then fed to the inner criterion, the proper
		// candidate index matching between B and B' is the composition of
		// the elimination, reversed, and the inner criterion.
		candidate_reordering = cp_tools::compose(cp_tools::reverse(
					eliminator.get_candidate_reordering()),
				inner_criterion_in->get_candidate_reordering(), true);
	} else {
		assert(numcands_before == inner_criterion->get_numcands_before());
		assert(numcands_after == eliminator.get_numcands_after());

		if (inner_criterion_in->get_numcands_after() !=
			elimination_spec.size()) {

			throw std::runtime_error("ISDA: candidate number mismatch"
				" in criterion composition!");
		}

		// The proper candidate index matching between B and B' is the
		// composition of the rearrangement performed by the inner
		// criterion (since it is called first),and the elimination.
		candidate_reordering = cp_tools::compose(
				inner_criterion_in->get_candidate_reordering(),
				eliminator.get_candidate_reordering(), true);
	}
}