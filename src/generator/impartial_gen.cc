// Ballot generator for generalized impartial culture. See header.

#include "ballotgen.h"
#include "impartial_gen.h"
#include <list>


std::list<ballot_group> impartial_gen::generate_ballots_int(int num_voters,
	int numcands, bool do_truncate, rng & random_source) const {

	// Fill a vector with 0...numcands, then shuffle randomly, then turn
	// into ordering. The reason we don't use generate_ordering
	// (indiv_ballot_gen) is that if we were to, we'd have to do the
	// allocation and iota every single time, which would really eat up
	// the cycles.

	std::vector<int> candidates(numcands, 0);
	iota(candidates.begin(), candidates.end(), 0);

	std::list<ballot_group> toRet;
	ballot_group to_add;
	to_add.set_weight(1);

	// Equal rank has been disabled and truncation is a hack. Implement
	// them properly once I know how. (Truncation: perhaps by choosing a
	// subset of candidates uniformly at random and then ranking them in
	// random order?)
	std::vector<bool> equal_rank(numcands);

	double total_weight = 0;

	while (total_weight < num_voters) {
		to_add.contents.clear();

		random_shuffle(candidates.begin(), candidates.end(),
			random_source);

		size_t this_far;
		if (do_truncate && candidates.size() > 1) {
			this_far = random_source.irand(1, candidates.size());
		} else	{
			this_far = candidates.size();
		}

		// HACK, part II
		// The proper way to do this is to build randomly, i.e. every
		// possible ballot with truncation/equality/etc is equally
		// likely. But *how*?
		bool do_equal_rank = false;

		int lin_counter = 0;

		for (size_t counter = 0; counter < this_far; ++counter) {
			to_add.contents.insert(candscore(candidates[counter],
					lin_counter));
			if (counter == 0 || !do_equal_rank ||
				!equal_rank[counter-1]) {
				++lin_counter;
			}
		}

		to_add.set_weight(get_sample(random_source));
		total_weight += to_add.get_weight();
		toRet.push_back(to_add);
	}

	return (toRet);
}
