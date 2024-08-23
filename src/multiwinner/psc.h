#pragma once

#include "singlewinner/pairwise/simple_methods.h"
#include "common/ballots.h"
#include "helper/dsc.cc"
#include "methods.h"
#include <list>

// TODO: User can spec. DSC, DAC, and HDSC. Makes no difference yet because
// all ballots are complete.

// TODO: Figure out why the map of sets become so extremely large. After all,
// it shouldn't be of greater size than (1 + 2 + 3 + 4 + .. n) * n. But
// the above is 0.5n^2, thus we have n^3, which could explain it. A trie could
// do better.

class PSC : public multiwinner_method {

	private:
		std::list<int> get_council(double C, double divisor,
			int council_size, int num_candidates,
			const election_t & ballots) const;

	public:
		std::list<int> get_council(int council_size, int num_candidates,
			const election_t & ballots) const;

		std::string name() const {
			return "PSC-CLE";
		}

};