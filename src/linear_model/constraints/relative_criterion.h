#pragma once

#include <vector>
#include <map>

#include "../lin_relation/constraint_set.h"
#include "../../tools/tools.h"

// Constraints that involve relative criteria.

// A relative criterion is a criterion that says that if the ballots change
// in some way, then some property must hold - usually that the winner must
// be the same. E.g. for mono-raise, the ballots may change by raising the
// winner, and the winner should stay the same; or for mono-add-top, the
// winner must stay the same even when some ballots ranking that candidate
// first are added.

// This class of constraint generator binds together two settings ("before"
// and "after"), specifying what transformations may be done to go from
// "before" to "after". Note that it doesn't implement a relative criterion
// in itself; it's just used to generate relative criterion examples
// through constraint programming.

// For now only handles candiate- and voter-preserving transitions. Fix
// later. Also make things like ISDA a relative criterion ("if we eliminate
// non-Smith candidates, the winner shouldn't change").

class relative_criterion_const {
	private:
		void add_addition_removal_terms(relation_side & add_where,
			std::string after_suffix,
			const std::vector<int> & cur_permutation) const;
		std::map<std::vector<int>, std::vector<std::vector<int> > >
			get_after_from_before_transitions() const;
		std::map<std::vector<int>, std::vector<std::vector<int> > >
			reverse_map(const std::map<std::vector<int>,
				std::vector<std::vector<int> > > & in) const;
		// Get a set of constraints that define the after-ballots in terms
		// of the before-ballots and transition counts.
		constraint_set get_after_constraints(const
			std::map<std::vector<int>, std::vector<std::vector<int> > > &
			after_from_before_transitions, std::string before_suffix,
			std::string after_suffix) const;

		// Get a set of constrants that define the before-ballots in terms
		// of the after-ballots and transition counts.
		constraint_set get_before_constraints(const
			std::map<std::vector<int>, std::vector<std::vector<int> > > &
			before_to_after_transitions, std::string before_suffix,
			std::string after_suffix) const;

		// Get a constraint that links the before ballots to the after
		// ballots with equality.
		constraint get_before_after_equality(
			std::string before_suffix, std::string after_suffix) const;

		// Get the default after_as_before. Remember to call *after*
		// numcands has been set.
		std::vector<int> get_default_after_as_before() const;

	protected:
		size_t numcands_before, numcands_after;

		// Some relative criteria alter what candidates are in play.
		// For these, the vector after_as_before is an index where
		// after_as_before[i] gives what after candidate corresponds to what
		// before candidate.
		// E.g. after_as_before = {0, 0, 1, 2} means that we're turning
		// three candidates into four, and the after candidates A, B, C, D
		// correspond to the before candidates A, B, C, respectively.
		std::vector<int> after_as_before;

		virtual bool is_valid_numcands_combination() const {
			return numcands_before == numcands_after; }

		// Default is passthrough for transition, and nothing at all for
		// addition and deletion.
		virtual bool permissible_transition(
			const std::vector<int> & before_permutation,
			const std::vector<int> & after_permutation) const {
			return before_permutation == after_permutation; }
		virtual bool permissible_addition(
			const std::vector<int> & permutation) const { return false; }
		virtual bool permissible_deletion(
			const std::vector<int> & permutation) const { return false; }

	public:

		// States whether the criteria requires that A's position is not
		// harmed (the default for monotonicity), that A's position is not
		// helped (immunity to teaming) or both (clone independence).
		virtual bool no_harm() const { return true; }
		virtual bool no_help() const { return false; }

		virtual std::string name() const = 0;

		constraint_set relative_constraints(std::string before_suffix,
			std::string after_suffix) const;

		size_t get_before_cand_number(size_t after_cand_number) const {
			assert(after_cand_number < after_as_before.size());
			return after_as_before[after_cand_number];
		}

		size_t get_numcands_before() const { return numcands_before; }
		size_t get_numcands_after() const { return numcands_after; }

		relative_criterion_const(size_t numcands_before_in,
			size_t numcands_after_in) {

			numcands_before = numcands_before_in;
			numcands_after = numcands_after_in;

			after_as_before = get_default_after_as_before();
		}

		relative_criterion_const(size_t numcands_in) :
			relative_criterion_const(numcands_in, numcands_in) {}

		virtual ~relative_criterion_const() {}
};