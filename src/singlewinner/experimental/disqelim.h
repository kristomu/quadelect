#pragma once

// Disqualified elimination: this takes an input order and repeatedly eliminates
// the lowest ranked candidate (by that order) who doesn't disqualify someone
// else on any subset of the sub-election containing the currently continuing
// candidates.

// Other ideas may be possible, e.g. if the current loser disqualifies someone,
// try whoever he disqualifies instead; follow the chain down to someone who
// doesn't disqualify anyone and then eliminate him. (If he disqualifies multiple,
// pick the lowest-ranked.) But one thing at a time.

// Like rmr1, a lot of the code here will be cribbed from elsewhere. I'll clean
// it up if necessary.

// (A inclusively disqualifies B on a subelection iff A disqualifies B on
// that subelection and every subset of it containing both A and B.)

// ---

// I thought this method was monotone if the base method is strongly monotone
// (i.e. raising A doesn't change the relative order of the other candidates).
// But that's false -- if the code below actually implements this method.

// It's most likely not pushover-proof either. But is it cloneproof?

#include "../method.h"
#include "pairwise/matrix.h"
#include "../positional/simple_methods.h"

#include <vector>
#include <memory>

class disqelim : public election_method {
	private:
		plurality plurality_method;
		bool has_method;
		std::shared_ptr<election_method> base_method;
		ordering base_ordering;

		// Returns true if the given candidate
		// inclusively disqualifies someone else, false
		// otherwise. Brute force approach.
		bool disqualifies(int candidate,
			const election_t & papers,
			const std::vector<bool> & continuing,
			const std::vector<bool> & hopefuls,
			int num_candidates) const;

	public:
		std::pair<ordering, bool> elect_inner(
			const election_t & papers,
			const std::vector<bool> & hopefuls,
			int num_candidates, cache_map * cache,
			bool winner_only) const;

		void set_base_ordering(ordering base_ordering_in) {
			base_ordering = base_ordering_in;
			has_method = false;
			base_method = NULL;
		}

		void set_base_method(std::shared_ptr<election_method>
			base_method_in) {

			has_method = true;
			base_method = base_method_in;
		}

		std::string name() const {
			if (has_method) {
				return "Disqualified elimination-[" + base_method->name() + "]";
			} else {
				return "Disqualified-elimination, fixed order";
			}
		}

		disqelim() : plurality_method(PT_FRACTIONAL) {}

		disqelim(std::shared_ptr<election_method> base_method_in) :
			plurality_method(PT_FRACTIONAL) {
			set_base_method(base_method_in);
		}
};
