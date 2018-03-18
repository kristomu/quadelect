// Monotonicity: Implements different types of mono-add criteria. These are:

// - Mono-add-plump: Adding further ballots that vote for X alone shouldn't make
//                   X lose.

// - Mono-add-top:   Adding further ballots that vote for X and are otherwise in
//                   random order shouldn't make X lose.

#include "mono_add.h"

// Other classes will be needed for mono-remove-bottom and "related" property, 
// Participation.

///// Mono-add-plump
////////////////////

bool mono_add_plump::add_ballots(const vector<int> & data,
		rng & randomizer, list<ballot_group> & input,
		double total_weight, int numcands) const {

	// Logarithmic hack for weight. If the random choice < 0.2, then 1.
	// Otherwise, random r = 0..9 and val = 1 - (ln(10-r)/l(10)) fraction
	// of total.
	
	double wt;
	if (randomizer.drand() < 0.2 && total_weight > 1)
		wt = 1;
	else	wt = total_weight * (1 - (log(10-randomizer.drand()*9)/
				log(10)));

	int cand = data[0];

	ordering plump;
	plump.insert(candscore(cand, 1));

	input.push_back(ballot_group(round(wt), plump, false, false));

	return(true);
}

///// Mono-add-top
//////////////////

// We might want to have it add multiple random ballots.. then on the other
// hand, the random input ballot could handle that...

bool mono_add_top::add_ballots(const vector<int> & data,
                rng & randomizer, list<ballot_group> & input,
                double total_weight, int numcands) const {

	// See above.

        double wt;
        if (randomizer.drand() < 0.5 && total_weight > 1)
                wt = round(randomizer.drand() * 10);
        else    wt = round(randomizer.drand() * total_weight);

	if (wt <= 0)
		return(false);

        int cand = data[0];

	// Generate a random ordering for the other candidates.
	// TODO: Use a ballot generator for this, and let it be specified.
	// Bluesky: Use same generator as the test.
	vector<int> other_candidates(numcands);
	iota(other_candidates.begin(), other_candidates.end(), 0);
	random_shuffle(other_candidates.begin(), other_candidates.end());

	// Build the actual ordering: First insert the top candidate
	// with higher score than any can match, then fill in the rest
	// (skipping cand itself).
        ordering o_rand;
        o_rand.insert(candscore(cand, numcands+1));

	for (size_t counter = 0; counter < other_candidates.size(); ++counter)
		if (other_candidates[counter] != cand)
			o_rand.insert(candscore(other_candidates[counter], 
						counter));

        input.push_back(ballot_group(round(wt), o_rand, false, false));

        return(true);
}
