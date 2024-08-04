// This is the base class for sets that are defined as the maximal elements
// with respect to some relation. That is, if the set consists of members who,
// for some relation X, there are no X from members outside the set onto members
// inside, and this set is the minimal for which that is true, the set generator
// can be modeled by extending this maximal elements class.

// The set-determining methods are classed as "election methods", but they
// produce ties so often that they really just elect sets.

#ifndef _SET_MAX_ELEMS
#define _SET_MAX_ELEMS

#include "singlewinner/method.h"
#include "singlewinner/pairwise/method.h"

class det_sets_relation {

	protected:
		virtual bool relation(const abstract_condmat & input, int a,
			int b, const std::vector<bool> & hopefuls) const = 0;

		// Is this - to have nested sets - required? Make it an
		// option when invoking these set methods.

		ordering nested_sets(const abstract_condmat & input,
			const std::vector<bool> & hopefuls, size_t limit) const;
		ordering nested_sets(const abstract_condmat & input,
			const std::vector<bool> & hopefuls) const {
			return (nested_sets(input, hopefuls,
						input.get_num_candidates()));
		}
};

#endif
