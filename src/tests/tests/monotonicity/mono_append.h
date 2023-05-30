// Monotonicity: Mono-append. A method fails it if altering some truncated
// ballots that do not include X by adding X to the end, makes X lose.

#ifndef _TWOTEST_MONO_MAP
#define _TWOTEST_MONO_MAP

#include "monotonicity.h"

// TODO: Fix infinite loop in the proximity of this
class mono_append : public monotonicity {

	private:
		bool alter_ballot(const ordering & input,
			ordering & output, int numcands,
			const std::vector<size_t> & data,
			rng & randomizer) const;

		std::string basename() const {
			return ("Mono-append");
		}

		bool allows_lowering() const {
			return (true);
		}

	public:

		mono_append(bool winner_only_in, bool permit_ties_in) :
			monotonicity(winner_only_in, permit_ties_in) {}

		mono_append() : monotonicity() {}

};

#endif
