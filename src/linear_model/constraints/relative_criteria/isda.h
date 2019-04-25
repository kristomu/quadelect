#pragma once

#include "../relative_criterion.h"
#include "elimination.h"
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

// The before ballot is considered a ballot after Smith losers have been
// eliminated. The inner criterion takes this ballot, before elimination,
// as input and produces some output after e.g. raising a candidate on it.
// In other words:

// inner criterion's before ballot -> pairwise constraints ->
// elimination by the given schedule -> before ballot (fewer candidates)

// inner criterion's before ballot -> inner criterion ->
// after ballot (more candidates)

// and if after, the simpler:

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
	private:
		constraint_set smith_loser_constraints(std::string ballot_suffix,
			std::string description_suffix) const;

		// Determine the number of candidates based on how many candidates
		// the inner criterion accepts and returns, and on the elimination
		// schedule.
		size_t calc_num_before_cands(bool elimination_first,
			std::shared_ptr<relative_criterion_const> &
			inner_criterion_in, const std::vector<int> &
			elimination_spec_in) const;

		size_t calc_num_after_cands(bool elimination_first,
			std::shared_ptr<relative_criterion_const> &
			inner_criterion_in, const std::vector<int> &
			elimination_spec_in) const;

		void print_cand_pairs(const cand_pairs & in) const;
		cand_pairs compose(const cand_pairs & first,
			const cand_pairs & second) const;
		cand_pairs reverse(const cand_pairs & to_reverse) const;

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

		// We need the fully-formed inner criterion to make any
		// judgments about whether numcands_before and numcands_after
		// make any sense, so be explicit about it. UGLY HACK.
		// Refactor later!
		isda_relative_const(size_t numcands_before_in,
			size_t numcands_after_in) : relative_criterion_const(
			numcands_before_in, numcands_after_in), eliminator(
			numcands_before_in, numcands_after_in) {

			throw std::logic_error("not supported");
		}

		isda_relative_const(size_t numcands) : isda_relative_const(
			numcands, numcands) {}

		// due to after_as_before, we must have at least as many after
		// candidates as befores. Somehow handle the opposite case in a
		// way that producer can deal with, then go from after_as_before
		// to fully fledged pairs.
		isda_relative_const(bool elimination_first,
			std::shared_ptr<relative_criterion_const> inner_criterion_in,
			std::vector<int> elimination_spec_in);
};