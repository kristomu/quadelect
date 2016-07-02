// Ballot generator for impartial culture. See header.

#include "ballotgen.h"
#include "impartial.h"
#include <list>

using namespace std;

list<ballot_group> impartial::generate_ballots_int(int num_voters, int numcands,
		bool do_truncate, rng & random_source) const {

	// Fill a vector with 0...numcands, then shuffle randomly, then turn
	// into ordering. The reason we don't use generate_ordering 
	// (indiv_ballot_gen) is that if we were to, we'd have to do the
	// allocation and iota every single time, which would really eat up
	// the cycles.
	
	vector<int> candidates(numcands, 0);
	iota(candidates.begin(), candidates.end(), 0);

	list<ballot_group> toRet;
	ballot_group to_add;
	to_add.weight = 1;

	// Equal rank has been disabled and truncation is a hack. Implement
	// them properly once I know how.
	vector<bool> equal_rank(numcands);

	for (int i = 0; i < num_voters; ++i) {
		to_add.contents.clear();

		random_shuffle(candidates.begin(), candidates.end(), 
				random_source);

		size_t this_far;
		if (do_truncate && candidates.size() > 1)
			this_far = random_source.irand(1, candidates.size());
		else	this_far = candidates.size();

		// HACK, part II
		// The proper way to do this is to build randomly, i.e. every
		// possible ballot with truncation/equality/etc is equally
		// likely. But *how*?
		bool do_equal_rank = false;
		/*bool do_equal_rank = (random() % 5 == 0) && 
			(candidates.size() > 2);
		do_equal_rank = false;
		if (do_equal_rank) {
			size_t to_equal_rank = 1 + random() % 
				(candidates.size()-1);
			fill(equal_rank.begin(), equal_rank.begin() +
					to_equal_rank, true);
			fill(equal_rank.begin() + to_equal_rank,
					equal_rank.end(), false);

			random_shuffle(equal_rank.begin(), equal_rank.end());
		}*/

		int lin_counter = 0;

		for (int counter = 0; counter < this_far; ++counter) {
			to_add.contents.insert(candscore(candidates[counter], 
						lin_counter));
			if (counter == 0 || !do_equal_rank || 
					!equal_rank[counter-1])
				++lin_counter;
		}

		toRet.push_back(to_add);
	}

	return(toRet);
}
