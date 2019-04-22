#include "isda.h"

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
	if (isda_before) {
		return "ISDA-before(" + inner_criterion->name() + ")";
	} else {
		return "ISDA-after(" + inner_criterion->name() + ")";
	}
	assert (1 != 1);
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

	// Number of candidates before must not be greater than number of
	// candidates after, due to limitations which will be handled later.
	// (It might actually work; I'll check that later.)

	if (numcands_before > numcands_after) {
		throw std::runtime_error("ISDA: numcands_before > numcands_after");
	}

	// Verify that the number of candidates prior to elimination match.
	if (isda_before) {
		assert (inner_criterion_in->get_numcands_before() ==
			elimination_spec.size());
	} else {
		assert (inner_criterion_in->get_numcands_after() ==
			elimination_spec.size());
	}

	// Construct after_as_before.

	// elimination_spec[x] is -1 if the xth candidate is eliminated,
	// otherwise the number that candidate corresponds to after, e.g.
	// eliminating B in a  4cddt gives {0, -1, 1, 2};

	after_as_before.resize(numcands_after);

	for (size_t i = 0; i < numcands_after; ++i) {
		// Mark as not assigned.
		after_as_before[i] = numcands_before+1;

		for (size_t j = 0; j < elimination_spec.size(); ++j) {
			// BEWARE: signed/unsigned comparison! Not ideal.
			if (elimination_spec[j] != (int)i) { continue; }

			// Check if we've already set after_as_before for candidate i.
			// If so, we have a bug.
			assert (after_as_before[i] == (int)(numcands_before+1));

			after_as_before[i] = j;
		}
	}
}