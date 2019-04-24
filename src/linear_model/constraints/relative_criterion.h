#pragma once

#include <numeric>
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
// and "after"). Note that it doesn't implement a relative criterion
// in itself; it's just used to generate relative criterion examples
// through constraint programming.

// This is the abstract base class. Some relative criterion constraints
// can be further categorized - see e.g. direct_relative_criteria.

// ----

// If cand_pairs[x] contains y, then that means that candidate index x
// before modification corresponds to candidate index y after. A given
// candidate before may correspond to many after (e.g. when cloning)
// or none (when eliminating that candidate).
typedef std::vector<std::vector<size_t> > cand_pairs;

class relative_criterion_const {
	private:

		// Get the default corresopndence between candidate numbers for
		// B and B'. The default is just the identity: that each choice of
		// index for candidate B (before alteration) is preserved. Anything
		// that does elimination will have to change this.
		cand_pairs get_default_candidate_reordering() const {

			cand_pairs out;
			for (size_t i = 0; i < numcands_before; ++i) {
				out.push_back(std::vector<size_t>(1, i));
			}
			return out;
		}

	protected:
		size_t numcands_before, numcands_after;

		// Some relative criteria alter what candidates are in play.
		// For these, the vector after_as_before is an index where
		// after_as_before[i] gives what after candidate corresponds to what
		// before candidate.
		// E.g. after_as_before = {0, 0, 1, 2} means that we're turning
		// three candidates into four, and the after candidates A, B, C, D
		// correspond to the before candidates A, B, C, respectively.
		// TODO: refactor this to pairs.
		cand_pairs candidate_reordering;

		virtual bool is_valid_numcands_combination() const {
			return numcands_before == numcands_after; }

	public:

		// States whether the criterion requires that A's position is not
		// harmed (the default for monotonicity), that A's position is not
		// helped (immunity to teaming) or both (clone independence).
		virtual bool no_harm() const { return true; }
		virtual bool no_help() const { return false; }

		virtual std::string name() const = 0;

		virtual constraint_set relative_constraints(std::string before_suffix,
			std::string after_suffix) const = 0;

		const cand_pairs & get_candidate_reordering() const {
			return candidate_reordering;
		}

		size_t get_numcands_before() const { return numcands_before; }
		size_t get_numcands_after() const { return numcands_after; }

		relative_criterion_const(size_t numcands_before_in,
			size_t numcands_after_in) {

			numcands_before = numcands_before_in;
			numcands_after = numcands_after_in;

			candidate_reordering = get_default_candidate_reordering();
		}

		relative_criterion_const(size_t numcands_in) :
			relative_criterion_const(numcands_in, numcands_in) {}

		virtual ~relative_criterion_const() {}
};