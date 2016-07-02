// Monotonicity: Implements different types of mono-sub criteria. These are:

// - Mono-sub-plump: Replacing some ballots that do not vote X top with ballots
//                   that vote for X alone shouldn't make X lose.

// - Mono-sub-top:   Replacing some ballots that do not vote X top with ballots
//                   that vote X top (and are otherwise arbitrary) shouldn't
//                   make X lose.

#ifndef _TWOTEST_MONO_MS
#define _TWOTEST_MONO_MS

#include "monotonicity.h"

///// Mono-sub-plump
////////////////////

class mono_sub_plump : public monotonicity {

        private:
                bool alter_ballot(const ordering & input,
                                ordering & output, int numcands,
                                const vector<int> & data,
                                rng & randomizer) const;

                string basename() const { return("Mono-sub-plump"); }

		// Not strictly true, but I'm lazy.
                bool allows_lowering() const { return(false); }

        public:

                mono_sub_plump(bool winner_only_in, bool permit_ties_in) :
                        monotonicity(winner_only_in, permit_ties_in) {}

                mono_sub_plump() : monotonicity() {}

};

///// Mono-sub-top
//////////////////

class mono_sub_top : public monotonicity {

        private:
                bool alter_ballot(const ordering & input,
                                ordering & output, int numcands,
                                const vector<int> & data,
                                rng & randomizer) const;

                string basename() const { return("Mono-sub-top"); }

                // Not strictly true, but I'm lazy.
                bool allows_lowering() const { return(false); }

        public:

                mono_sub_top(bool winner_only_in, bool permit_ties_in) :
                        monotonicity(winner_only_in, permit_ties_in) {}

                mono_sub_top() : monotonicity() {}

};

#endif
