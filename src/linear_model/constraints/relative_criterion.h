#pragma once

#include <vector>
#include <map>

#include "../lin_relation/constraint_set.h"

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

	protected:
		int numcands_before, numcands_after;

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

		constraint_set relative_constraints(std::string before_suffix,
			std::string after_suffix) const;

		relative_criterion_const(int numcands_in) {
			numcands_before = numcands_in;
			numcands_after = numcands_in;
		}

		relative_criterion_const(int numcands_before_in, 
			int numcands_after_in) {
			numcands_before = numcands_before_in;
			numcands_after = numcands_after_in;
		}

		virtual ~relative_criterion_const() {}
};