// Schulze STV.

// Added minimal shunt functions to enable this for thirdelect usage, although
// it may be too slow. -KM

// TODO: Fix unexplained crash that appears after a few rounds. It's probably
// related to the archaic allocation practice of the system, or the global
// variables that retain their value between rounds.
// It happens even when we start from the round in question, so I guess it's
// a memory allocation problem, and not any retention problem.
//		Do this again now that I have greater debugging skills? :-P

#include "methods.h"
#include <list>

class SchulzeSTV : public multiwinner_method {

	public:
		std::list<int> get_council(int council_size,
			int num_candidates, const election_t & ballots) const;

		std::string name() const {
			return ("Schulze STV");
		}

		// HACK - not really true.
		// Ideally should be two functions: first, "is permissible for
		// all", second, "is permissible for this particular setup"
		// (that is, numcands, council size, etc.)
		// The former would be used for one-char info so that those that
		// only sample a subset are marked as such.

		// Or perhaps "Expected time" and "expected space", with -1
		// on both for "always acceptable".
		bool polytime() const {
			return (false);
		}

};