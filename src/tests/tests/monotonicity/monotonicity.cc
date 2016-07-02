// Base class for monotonicity problems. Monotonicity criteria involve doing
// something that seems to be beneficial to a candidate; the method then fails
// if doing so makes the candidate lose (if the test is winner-only) or lowers
// his rank (if the test is full-rank), or conversely, if doing something that
// seems harmful to the candidate makes him win.

#include "monotonicity.h"

using namespace std;

// Abstract class for monotonicity type criteria. The only virtual and complex
// alterable function is the one that modifies a given ballot.

ordering::const_iterator monotonicity::find_cand(const ordering & to_search,
		int candnum) const {
	ordering::const_iterator place_cand;

	for (place_cand = to_search.begin(); place_cand != to_search.end() 
			&& place_cand->get_candidate_num() != candnum;
			++place_cand);

	return(place_cand);
}

vector<int> monotonicity::generate_aux_data(const list<ballot_group> & input, 
		int numcands) const {

	assert (!input.empty());

	// Yuck - linear time.
	int num_ballots = input.size();
	int orders_to_modify = 0;
	if (num_ballots > 1)
		orders_to_modify = 1 + random() % (num_ballots-1);

	vector<bool> to_alter(num_ballots, false);

	int counter = 0;

	vector<int> output(2 + orders_to_modify + 1, 0);

	while (counter < orders_to_modify) {
		int possible = random() % to_alter.size();
		possible = counter; // <--- FIX?
		if (!to_alter[possible]) {
			to_alter[possible] = true;
			output[2 + counter++] = possible;
		}
	}

	output[0] = random() % numcands;

	if (allows_lowering())
		output[1] = random() % 2;
	else	output[1] = 1; // always raise

	*(output.rbegin()) = random(); // seed

	return(output);
}

pair<bool, list<ballot_group> > monotonicity::rearrange_ballots(
		const list<ballot_group> & input,
		int numcands, const vector<int> & data) const {

	int seed = *(data.rbegin());

	// 32 bit should be good enough
	rng randomizer(seed);

	int counter = -1, ballot_idx = 0;

	pair<bool, list<ballot_group> > output;
	output.first = false;

	double total_weight = 0;

	for (list<ballot_group>::const_iterator pos = input.begin(); pos !=
			input.end(); ++pos) {
		total_weight += pos->weight;

		++counter;
		// If that isn't the ballot we're looking for, move along.
		if (counter != data[ballot_idx + 2]) {
			// If there are no more ballots to alter, and we
			// didn't manage to raise any, no point continuing.
			// NOTE: since the ballots are random, we can choose
			// to only modify the first p (for some p)! That will
			// be much quicker! Do that.
			if (counter > data[ballot_idx + 2] && !output.first)
				return(output);
			output.second.push_back(*pos);
		       	continue;
		}

		// Okay, this one should be changed.
		++ballot_idx;
		ordering order_in, order_out;
		bool raise = (data[1] == 1);

		if (raise) // raise
			order_in = otools.scrub_scores(pos->contents);
		else	order_in = otools.scrub_scores(otools.reverse(
					pos->contents));

		// Determine share of voters that should go to the modified
		// part in order to handle compressed ballots. This should
		// always be at least one.
		// TODO: Make reproducible. Data should always have a random
		// seed.
		int modified = 1;
		if (pos->weight >= 2) {
			modified += round(drand48() * (pos->weight - 1));
		}

		bool altered = alter_ballot(order_in, order_out, numcands, 
				data, randomizer);

		// Reverse back if we did reverse it to collapse lower to
		// raise.
		if (altered) {
			if (!raise)
				order_out = otools.scrub_scores(otools.
						reverse(order_out));

			output.second.push_back(ballot_group(modified,
						order_out, pos->complete,
						false));
			if (modified < pos->weight)
				output.second.push_back(ballot_group(
							pos->weight - modified,
							pos->contents, 
							pos->complete,
							false));

			output.first = true;
		} else 
			output.second.push_back(*pos);
	}

	// Add ballots if it's mono-add-top etc. Or the output.first with the
	// result of adding ballots.
	
	// HACK HACK. TODO, FIX.
	int fraction = total_weight;
	while (total_weight/(double)fraction < 1)
		fraction = round(total_weight * randomizer.drand());
	if (fraction == 0) ++fraction;

	for (int counter = 0; counter < fraction; ++counter)
		output.first |= add_ballots(data, randomizer, output.second,
				total_weight/(double)fraction, numcands);
	
	return(output);
}

bool monotonicity::applicable(const ordering & check,
		const vector<int> & data, bool orig) const {

	ordering::const_iterator pos;

	if (!permit_ties) {
               // If there's any tie at all, no cookie for you!
                double delta = check.begin()->get_score();
                for (pos = check.begin(); pos != check.end(); ++pos) {
                        if (pos == check.begin()) continue;
                        if (pos->get_score() == delta) return(false);
                        delta = pos->get_score();
                }
        }

	// If this is the original ballot, we're raising, and only considering
	// the winner, then check if the designated candidate is at top. If not,
	// the method must pass because a loser can't be made loseringer [sic].
	if (orig && (data[1] == 1) && winner_only()) {
		int cand = data[0];

		bool found = false;

		for (pos = check.begin(); pos != check.end() && pos->get_score()
				== check.begin()->get_score() && !found; ++pos)
			if (pos->get_candidate_num() == cand)
				found = true;

		if (!found) return(false);
	}

	return(true);
}

bool monotonicity::pass_internal(const ordering & original, const ordering &
		modified, const vector<int> & data, int numcands) const {

	// Get the candidate we have raised/lowered.
	int cand = data[0];
	bool raise = (data[1] == 1);

	// Get the scores, normalized so ratings don't affect anything.
	// (Unnormalized could be interesting, too, for those methods that
	//  provide ratings!)
	
	ordering scrub_orig, scrub_mod;
	
	scrub_orig = otools.scrub_scores_by_cand(original);
	scrub_mod = otools.scrub_scores_by_cand(modified);
		
	ordering::const_iterator a_place_cand, b_place_cand;

	// Find the position.
	a_place_cand = find_cand(scrub_orig, cand);
	b_place_cand = find_cand(scrub_mod, cand);

	// This shouldn't happen, but if it does, return true (can't judge)
	if (a_place_cand == scrub_orig.end() || b_place_cand == scrub_mod.end())
		assert(1 != 1);
	//	return(true);

	// If we're only interested in the winner, check whether the candidate
	// in question is in the winner set of each.
	bool a_winner = a_place_cand->get_score() == 
		scrub_orig.begin()->get_score();

	bool b_winner = b_place_cand->get_score() ==
		scrub_mod.begin()->get_score();

	bool a_loser = a_place_cand->get_score() ==
		scrub_orig.rbegin()->get_score();

	bool b_loser = b_place_cand->get_score() ==
		scrub_mod.rbegin()->get_score();

	// If it's a tie between all candidates, then it's impossible
	// to say whether there was a monotonicity violation. For instance,
	// say the original was A = B = C and C was raised, and then
	// A = B > C. If (A = B = C) is a shared first place, then that is
	// a violation, but if (A = B = C) is shared last, then it's not.
	// May take this down later.
	//if ((a_loser && a_winner) || (b_loser && b_winner))
	//	return(true);

	bool winner_passes;
	if (raise)
		winner_passes = !a_winner ||  b_winner; 
	else	winner_passes =  a_winner || !b_winner;

	// If the winner doesn't pass, we know it's nonmonotonic. Otherwise,
	// if we're only interested in winners, return the winner_passes
	// boolean anyhow, but if not, check the rest of the struct.
	if (!winner_passes)
		return(false);
	if (winner_only())
		return(winner_passes);

	double a_score = a_place_cand->get_score(), 
	       b_score = b_place_cand->get_score();

	bool preliminary_nonmonotonic = (raise && (a_score > b_score) ||
			!raise && (a_score < b_score));
	bool nonmonotonic = false;

	// Need to find a way of defining "raise" for a social ordering with
	// ties.
	// HACK
	if (preliminary_nonmonotonic) {
		ordering scrub_orig_rev, scrub_mod_rev;

		scrub_orig_rev = otools.scrub_scores_by_cand(otools.reverse(
					original));
		scrub_mod_rev = otools.scrub_scores_by_cand(otools.reverse(
					modified));

		ordering::const_iterator arpc = find_cand(scrub_orig_rev, 
				cand), brpc = find_cand(scrub_mod_rev, cand);

		a_score = -arpc->get_score();
	       	b_score = -brpc->get_score();
	}

	//if (raise && (a_score > b_score) || !raise && (a_score < b_score) ) {
	if (preliminary_nonmonotonic) {
		/*cout << "Intended candidate was " << cand << endl;
		cout << "Got score " << a_score << " from original, " << b_score << " from modified." << endl;
		cout << "Raise type is " << raise << endl;
		cout << "Orig:\t";
		ordering::const_iterator mah, maha;
		for (mah = scrub_orig.begin(); mah != scrub_orig.end(); ++mah)
			cout << "(" << mah->get_candidate_num() << ", " << mah->get_score() << ") ";
		cout << endl << "Mod:\t";
		for (maha = scrub_mod.begin(); maha != scrub_mod.end(); ++maha)
			                        cout << "(" << maha->get_candidate_num() << ", " << maha->get_score() << ") ";
		cout << endl << endl;
		if (raise) cout << "R"; else cout << "L";
		        if (a_winner) cout << "-aw-"; else cout << "-al-";
			        if (b_winner) cout << "bw" << endl; else cout << "bl" << endl;
*/
		return(false);
	}

	return(true);
}

string monotonicity::explain_change_int(const vector<int> & data,
		const map<int, string> & cand_names) const {

	int cand = data[0];
	bool raise = (data[1] == 1);

	assert (cand_names.find(cand) != cand_names.end());

	string event = cand_names.find(cand)->second + " was ";

	if (raise)
		event += "raised";
	else	event += "lowered";

	return(event);
}

string monotonicity::name() const {
        string base = basename() + "(";

        if (winner_only())
                base += "winner, ";
        else    base += "rank, ";

        if (permit_ties)
                base += "ties)";
        else    base += "pref)";

        return(base);
}
