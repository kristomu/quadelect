// Base class for positional voting systems. See positional.h.

#include "positional.h"
#include "aggregator.h"

#include <list>
#include <vector>

using namespace std;

// DONE: Handle equal ranks.
// Two options for rank: Equal rank, fractional rank. Two for truncated
// ballots: Tied at last, and modified Borda. BLUESKY: Implement that, too.

// Perhaps also a function that returns how many nonzero positions exist, so
// that it only counts the first votes for Plurality. DONE.

double positional::get_weight_sum(const list<ballot_group> & input) const {
	double weight_sum = 0;

	for (list<ballot_group>::const_iterator pos = input.begin(); pos !=
			input.end(); ++pos)
		weight_sum += pos->weight;

	return(weight_sum);
}

// Used to prepend the type to the name of the method. ER is equivalent to
// WHOLE, <> indicates FRACTIONAL.

string positional::show_type(const positional_type & kind_in) const {

	switch(kind_in) {
		case PT_WHOLE: return("ER-");
		case PT_FRACTIONAL: return("");
		default: return("??-");
	}
};

// TODO: Bail if it's of the wrong type, because the same ballots will give
// different positional matrices depending on whether we're ER or FR.
ordering positional::pos_elect(const vector<vector<double> > & matrix,
		int num_hopefuls, //double weight_sum,
		const vector<bool> * hopefuls) const {

	// Turn it into a social ordering.
	
	ordering social_order;

	for (int cand_num = 0; cand_num < matrix.size(); ++cand_num) {
		if (hopefuls != NULL)
			if (!(*hopefuls)[cand_num]) continue;

		double cand_score = 0;

		for (int sec = 0; sec < matrix[cand_num].size(); ++sec)
			cand_score += matrix[cand_num][sec] * pos_weight(
					sec, num_hopefuls - 1);

		social_order.insert(candscore(cand_num, cand_score));
	}

	return(social_order);
}

ordering positional::elect(const list<ballot_group> & input, 
		int num_candidates, int num_hopefuls, 
		const vector<bool> * hopefuls) const {

	return(pos_elect(positional_aggregator().get_positional_matrix(input,
					num_candidates, num_hopefuls, hopefuls,
					kind, zero_run_beginning()), 
				num_hopefuls, hopefuls));
}

pair<ordering, bool> positional::elect_inner(const list<ballot_group> & input,
		int num_candidates, cache_map & cache, bool winner_only) const {

	return(pair<ordering, bool>(
				elect(input, num_candidates, num_candidates, 
					NULL),
				false));
}

pair<ordering, bool> positional::elect_inner(const list<ballot_group> & input,
	       const vector<bool> & hopefuls, int num_candidates, 
	       cache_map & cache, bool winner_only) const {

	// Same as above, only pos_weight must be adjusted by however many
	// are excluded. We may have to alter the function to make that quick.
	// Or we could just go through hopefuls, but that's a waste.
	
	// In the spirit of YAGNI, we'll count here for now. If it gets too
	// bad, we'll eliminate it later.
	
	// We actually now count inside the main elect function, perhaps
	// just get the param from there...
	
	int num_hopefuls = 0;
	for (int counter = 0; counter < hopefuls.size(); ++counter)
		if (hopefuls[counter])
			++num_hopefuls;

	return(pair<ordering, bool>(
				elect(input, num_candidates, num_hopefuls, 
					&hopefuls),
				false));
};

double positional::get_pos_score(const ballot_group & input,
		int candidate_number, const vector<bool> * hopefuls, 
		int num_hopefuls) const {

	// Going down the ordering, increment a counter for each new rank.
	// Equal ranks split the first counter, but after they're investigated,
	// the counter increments as if they were strictly ranked.
	
	int counter = 0, span = 0;
	double lastvalue = INFINITY;

	for (ordering::const_iterator pos = input.contents.begin();
			pos != input.contents.end(); ++pos) {
		if (hopefuls != NULL)
			if (!(*hopefuls)[pos->get_candidate_num()])
				continue;

		if (pos->get_candidate_num() == candidate_number)
			return(pos_weight(counter, num_hopefuls - 1));

		++span;
		if (pos->get_score() < lastvalue) {
			counter += span;
			span = 0;
		}
	}

	return(-1);
}

double positional::get_pos_score(const ballot_group & input, 
		int candidate_number, int num_candidates) const {
	return(get_pos_score(input, candidate_number, NULL, num_candidates));
}

string positional::name() const {
	return (show_type(kind) + pos_name());
}
