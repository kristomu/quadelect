// Monotonicity: Implements variants on the mono-raise criterion. These are:

// - Mono-raise: a method fails it if altering some ballots by raising X makes
//               X lose.

// - Mono-raise-delete: a method fails it if raising X and truncating the ballot
//               after X, makes X lose.

// - Mono-raise-random: a method fails it if altering some ballots by raising X
//               and replacing all ranks lower than X's new rank with candidates
//               in random order, makes X lose.

#ifndef _TWOTEST_MONO_MR
#define _TWOTEST_MONO_MR

#include "monotonicity.h"

using namespace std;

///// Mono-raise
////////////////

class mono_raise : public monotonicity {

	private:
		bool alter_ballot(const ordering & input, ordering & output,
			size_t numcands, const vector<size_t> & data,
			rng & randomizer) const;

		string basename() const {
			return ("Mono-raise");
		}

		bool allows_lowering() const {
			return (true);
		}

	public:

		mono_raise(bool winner_only_in, bool permit_ties_in) :
			monotonicity(winner_only_in, permit_ties_in) {}

		mono_raise() : monotonicity() {}

};

///// Mono-raise-delete
///////////////////////

class mono_raise_delete : public monotonicity {

	private:
		bool alter_ballot(const ordering & input,
			ordering & output, size_t numcands,
			const vector<size_t> & data,
			rng & randomizer) const;

		string basename() const {
			return ("Mono-raise-delete");
		}

		bool allows_lowering() const {
			return (false);
		}

	public:

		mono_raise_delete(bool winner_only_in, bool permit_ties_in) :
			monotonicity(winner_only_in, permit_ties_in) {}

		mono_raise_delete() : monotonicity() {}

};

///// Mono-raise-random
///////////////////////

class mono_raise_random : public monotonicity {

	private:
		bool alter_ballot(const ordering & input,
			ordering & output, size_t numcands,
			const vector<size_t> & data,
			rng & randomizer) const;

		string basename() const {
			return ("Mono-raise-random");
		}

		bool allows_lowering() const {
			return (false);
		}

	public:

		mono_raise_random(bool winner_only_in, bool permit_ties_in) :
			monotonicity(winner_only_in, permit_ties_in) {}

		mono_raise_random() : monotonicity() {}

};

#endif
