#include "../relative_criterion.h"
#include "../pairwise.h"

#include <memory>

// This class implements independence of Smith-dominated alternatives over
// some other relative criterion. For instance, ISDA over mono-raise means
// that if one takes a starting election with a Smith set smaller than the
// number of candidates, eliminates non-Smith candidates, and raises the A
// candidate on some ballots, then the output should not make A lose where
// A won before these two operations were performed.

// The way it works is that it takes an inner relative criterion, as well
// as an elimination schedule and whether it's the before- or the after-
// election that has the ISDA reduction happen to it. (The example above
// is one where the ISDA reduction happens before the inner criterion.)

// It then composes the following set of constraints, if before:

// before ballot -> pairwise constraints -> elimination by the given
// schedule -> inner criterion -> after ballot

// and if after:

// before ballot -> inner criterion -> pairwise constraints -> elimination
// by the given schedule -> after ballot.

// The pairwise constraints specify that the candidates who are to be
// eliminated will lose pairwise to everybody who is not going to be
// eliminated. This will ensure that the constraints are not too strict,
// e.g. that mono-add-top can't get B into the Smith set if B is set to
// be eliminated. 

// As long as the scenarios used to set the before and after ballots have
// full Smith sets, that will keep uneliminated candidates from dropping out
// of the Smith set. Thus a candidate will be eliminated iff it's a Smith
// loser.

// Note that you only need to do reductions that eliminate a single
// candidate, because if the method obeys ISDA from 10 candidates to 9,
// and also from 9 to 8, it also obeys it from 10 to 8. (Is that true?
// BLUESKY: Prove.)

class isda_relative_const : public relative_criterion_const {
	public:
		std::shared_ptr<relative_criterion_const> inner_criterion;
		bool isda_before;
		// TODO: Deal with the inner criterion having different
		// before and after numcands.
		size_t numcands_inner_before, numcands_inner_after;

		elimination_util_const eliminator;
		std::vector<int> elimination_spec;

		constraint_set relative_constraints(
			std::string before_suffix, std::string after_suffix) const;

		bool no_harm() const { return inner_criterion->no_harm(); }
		bool no_help() const { return inner_criterion->no_help(); }

		std::string name() const;
};

std::string isda_relative_const::name() const {
	if (isda_before) {
		return "ISDA-before(" + inner_criterion->name() + ")";
	} else {
		return "ISDA-after(" + inner_criterion->name() + ")";
	}
}

constraint_set isda_relative_const::smith_loser_constraints(
	std::string suffix, int numcands) const {

	// Use the standard name for the margin of victory parameter; this
	// fixed parameter is set by test_generator.
	pairwise_constraints pwconst("min_victory_margin");

	constraint_set loser_constraints;

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
				winner, loser, suffix, numcands));
		}
	}

	return loser_constraints;
}

constraint_set isda_relative_const::relative_constraints(
	std::string before_suffix, std::string after_suffix) const {

	constraint_set isda_linkage;

	if (isda_before) {
		// before ballot -> pairwise constraints -> elimination by 
		// the given schedule -> inner criterion -> after ballot

		// Pairwise constraints on before_suffix
		isda_linkage.add(smith_loser_constraints(
			before_suffix, before_suffix + "_isda_loser", 
			numcands_before));

		// elimination
		isda_linkage.add(eliminator.relative_constraints(
			before_suffix, before_suffix + "_isda"));

		// Inner criterion
		isda_linkage.add(inner_criterion->relative_constraints(
			before_suffix + "_isda", after_suffix));
	} else {
		// before ballot -> inner criterion -> pairwise constraints ->
		// elimination by the given schedule -> after ballot.

		// Inner criterion
		isda_linkage.add(inner_criterion->relative_constraints(
			before_suffix, before_suffix + "_isda"));

		// Pairwise constraints on before_suffix + _isda go here
		isda_linkage.add(smith_loser_constraints(
			before_suffix + "_isda", before_suffix + 
			"_isda_isda_loser", numcands_before));

		// Elimination
		isda_linkage.add(eliminator.relative_constraints(
			before_suffix + "_isda", after_suffix));
	}

	return isda_linkage; // Now wasn't that easy?
}