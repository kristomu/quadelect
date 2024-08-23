// Hard Cardinal-type voting methods: Birational voting and LPV.
// These do an exhaustive search over the candidate space, and so can take a
// VERY long time for many candidates.

// Perhaps include minimax.

#pragma once

#include "methods.h"
#include <list>
#include <vector>

enum hardcard_type { HC_BIRATIONAL, HC_LPV };

struct scored_ballot {
	double weight; // #voters
	std::vector<double> scores; // #voters by #cands
};

struct hardcard_extrema {
	std::vector<bool> W_minimum_set, W_maximum_set;
	double W_minimum, W_maximum;
};

class hardcard : public multiwinner_method {

	private:
		hardcard_type type;

		std::vector<scored_ballot> make_cardinal_array(
			const election_t & ballots, int numcand) const;

		double birational(const std::vector<bool> & W, const
			scored_ballot & this_ballot) const;

		double birational(const std::vector<bool> & W, const std::vector<
			scored_ballot> & ballots) const;

		hardcard_extrema merge_extrema(const hardcard_extrema a,
			const hardcard_extrema b) const;

		hardcard_extrema all_birational(const std::vector<scored_ballot> &
			ballots, const std::vector<bool> & cur_W,
			std::vector<bool>::iterator begin,
			std::vector<bool>::iterator end,
			int marks_left) const;

		double LPV(const std::vector<bool> & W, int council_size,
			const scored_ballot &
			this_ballot, double k) const;

		double LPV(const std::vector<bool> & W, int council_size,
			const std::vector<scored_ballot>
			& ballots, double k) const;

		hardcard_extrema all_LPV(const std::vector<scored_ballot> &
			ballots, const std::vector<bool> & cur_W, double k,
			int council_size, std::vector<bool>::iterator begin,
			std::vector<bool>::iterator end,
			int marks_left) const;

	public:
		bool polytime() const {
			return false;    // something like this
		}

		std::list<int> get_council(int council_size, int num_candidates,
			const election_t & ballots) const;

		std::string name() const;

		hardcard(hardcard_type type_in) {
			type = type_in;
		}

};