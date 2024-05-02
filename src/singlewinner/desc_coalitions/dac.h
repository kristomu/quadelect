// Woodall's Descending Acquiescing Coalitions method.

// This is calculated just like DSC, except that instead of the score
// being based on the number of voters who are solidly committed to the
// set, it's based on the number of voters who acquiesce to that set.

// A voter acquiesces to a set of candidates if he ranks nobody outside
// the set higher than anybody inside the set.

// In particular, this means that equal-ranking a set of candidates bottom
// contributes to every subset of the equal-ranked candidates combined with
// the set of all candidates ranked above bottom.

// See the Descending Solid Coalitions header file for more information
// about the procedure.

// If equal-rank and truncation are both disallowed, DAC gives the same
// results as DSC.

// Figuring out who comes second might not be entirely correct yet, fix later.
// The following example by Benham:

// 46: A
// 44: B>C
// 10: C

// currently gives an outcome of C>A=B, but it seems sensible that
// it should distinguish between A and B.

#pragma once

#include "desc_coalition.h"

class dac : public desc_coalition_method {

	protected:
		std::vector<coalition_data> get_coalitions(
			const election_t & election,
			const std::vector<bool> & hopefuls,
			int numcands) const {

			return get_acquiescing_coalitions(election,
					hopefuls, numcands);
		}

	public:
		std::string name() const {
			return ("DAC");
		}
};