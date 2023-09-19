// Monotonicity: Implements different types of mono-add criteria. These are:

// - Mono-add-plump: Adding further ballots that vote for X alone shouldn't make
//                   X lose.

// - Mono-add-top:   Adding further ballots that vote for X and are otherwise in
//                   random order shouldn't make X lose.

#ifndef _TWOTEST_MONO_MA
#define _TWOTEST_MONO_MA

#include "monotonicity.h"

// Other classes will be needed for mono-remove-bottom and "related" property,
// Participation.

///// Mono-add-plump
////////////////////

class mono_add_plump : public monotonicity {

	private:
		bool add_ballots(const std::vector<int> & data,
			rng & randomizer,
			election_t & input,
			double total_weight, int numcands) const;

		std::string basename() const {
			return ("Mono-add-plump");
		}

		// More strictly true this time.
		bool allows_lowering() const {
			return (false);
		}

	public:

		mono_add_plump(bool winner_only_in, bool permit_ties_in) :
			monotonicity(winner_only_in, permit_ties_in) {}

		mono_add_plump() : monotonicity() {}

};

///// Mono-add-top
//////////////////

// We might want to have it add multiple random ballots.. then on the other
// hand, the random input ballot could handle that...

class mono_add_top : public monotonicity {

	private:
		bool add_ballots(const std::vector<int> & data,
			rng & randomizer,
			election_t & input,
			double total_weight, int numcands) const;

		std::string basename() const {
			return ("Mono-add-top");
		}

		// More strictly true this time.
		bool allows_lowering() const {
			return (false);
		}

	public:

		mono_add_top(bool winner_only_in, bool permit_ties_in) :
			monotonicity(winner_only_in, permit_ties_in) {}

		mono_add_top() : monotonicity() {}

};

#endif
