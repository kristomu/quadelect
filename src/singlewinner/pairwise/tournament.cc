// This one requires both an ordering and the Condorcet matrix, and therefore
// isn't a pairwise_method.

// Election method: Single elimination tournament. First seed the candidates
// according to the ordering given. Then pair up candidates, best against
// worst. The system is recursive, so that if you have 16 candidates, the 8
// that proceed are treated as the inputs to an 8-candidate method.

// Since a single run through the tournament structure provides exponentially
// many ties at each level (those who lasted until that level), we need to run
// "loser brackets" to determine the ordering beyond first place. This adds up
// to log_2(n) runs.

// It throws an exception on a tie.

// TODO (BLUESKY): In the same vein, have Kemenize, Insertion-sort-ize,
// mergesort, etc. Init according to seeding, then sort based on the method in
// question, then return.

#include "../../pairwise/matrix.h"
#include "../method.h"
#include "method.h"
#include "tournament.h"

#include <stdexcept>
#include <iostream>

using namespace std;

int cond_tournament::a_beats_b(int a, int b,
	const condmat & to_check) const {

	// First check if we're dealing with byes
	if (a >= to_check.get_num_candidates() ||
		b >= to_check.get_num_candidates()) {
		if (a >= to_check.get_num_candidates() &&
			b >= to_check.get_num_candidates()) {
			return (0);    // tie - shouldn't happen
		}
		if (a >= to_check.get_num_candidates()) {
			return (-1);    // b wins
		}
		return (1); // a wins
	}

	if (to_check.get_magnitude(a, b) > 0) {
		return (1);
	}
	if (to_check.get_magnitude(b, a) > 0) {
		return (-1);
	}
	return (0);
}

int cond_tournament::get_victor(int check_a, int check_b, int level, int
	numcands, const vector<int> & seed_order, const condmat &
	to_check, vector<int> & lasted_how_long) const {
	// If we're at the level so that 2^level >= numcands, then we do a
	// simple Condorcet check to determine which candidate wins. Otherwise,
	// we recurse down to the next level.

	cout << "level " << level << ": " << check_a << " vs " << check_b << endl;

	int num_at_level = (2 << level);

	cout << "num at level: " << num_at_level << " out of " << numcands << endl;

	int resolv_a, resolv_b;

	if (num_at_level >= numcands) {

		// If we're at the last level, it's easy enough: just get the
		// candidates corresponding to the seeding order.

		resolv_a = check_a, resolv_b = check_b;

		cout << "entering test level" << endl;

		if (resolv_a < numcands) {
			resolv_a = seed_order[check_a];
		}
		if (resolv_b < numcands) {
			resolv_b = seed_order[check_b];
		}

		cout << check_a << "(" << resolv_a << ")\t" <<
			check_b << "(" << resolv_b << ")" << endl;
	} else {
		// Otherwise, the candidates are whoever wins the next level
		// down.

		int neg_a = (num_at_level * 2) - (check_a + 1);
		int neg_b = (num_at_level * 2) - (check_b + 1);

		cout << "Recursion ops: " << check_a << " vs " << neg_a << endl;
		cout << "Recursion ops: " << check_b << " vs " << neg_b << endl;

		resolv_a = get_victor(check_a, neg_a,
				level+1, numcands, seed_order, to_check,
				lasted_how_long);
		resolv_b = get_victor(check_b, neg_b,
				level+1, numcands, seed_order, to_check,
				lasted_how_long);
	}

	cout << "level " << level << ": " << "Checking who wins of " << resolv_a <<
		" vs " << resolv_b << endl;

	switch (a_beats_b(resolv_a, resolv_b, to_check)) {
		default:
		case 0: cout << "Tie." << endl;
			// Dunno what kind of tiebreaker to implement here...
			throw std::runtime_error("Condorcet tournament: got a tie "
				"between two contestants, but method has no tiebreaker.");
			break;
		case 1: cout << "First guy won. " << endl;
			if (resolv_b < numcands) {
				lasted_how_long[resolv_b] = level;
			}
			return (resolv_a);
		case -1: cout << "Second guy won." << endl;
			if (resolv_a < numcands) {
				lasted_how_long[resolv_a] = level;
			}
			return (resolv_b);
	}
}

ordering cond_tournament::internal_elect(const list<ballot_group> & papers,
	const condmat &	matrix, const ordering & seed,
	const vector<bool> & hopefuls, int num_candidates,
	bool winner_only) const {

	// First determine the candidates that are still in the running, and
	// sort them in seed order. Then ask who wins of 0 against 1. This
	// provides a vector that tells at what level each candidate was
	// removed. Finally, turn that into an ordering.

	vector<int> seed_order(num_candidates, -1),
		   lasted_how_long(num_candidates, -2);
	size_t counter = 0;
	double old_cand_score = 0;

	// Count the number of hopefuls
	size_t num_hopefuls = 0;
	for (counter = 0; counter < hopefuls.size(); ++counter)
		if (hopefuls[counter]) {
			++num_hopefuls;
		}
	counter = 0;

	// TODO: interleave this. If the base method returns A > B > C > D,
	// then we should pit A against D and B against C, not A against B
	// and C against D.
	for (ordering::const_iterator pos = seed.begin(); pos != seed.end();
		++pos) {
		if (hopefuls[pos->get_candidate_num()]) {
			seed_order[counter++] = pos->get_candidate_num();
		}

		// If equal rank, break on assert
		// The counter != 0 is so we don't get a false positive for
		// the first candidate.
		if (counter != 0 && pos->get_score() != old_cand_score) {
			throw std::runtime_error("Condorcet tournament: got a tie "
				"preparing the brackets, but method has no tiebreaker.");
		}
		old_cand_score = pos->get_score();
	}

	// Check that we processed every hopeful candidate.
	// Perhaps this should be an assert? I should check later when to
	// use assertions.
	if (counter != num_hopefuls) {
		throw std::logic_error("Condorcet tournament: did not process"
			"every candidate when creating brackets!");
	}

	// SIDE EFFECT
	int victor = get_victor(0, 1, 0, num_hopefuls, seed_order, matrix,
			lasted_how_long);
	lasted_how_long[victor] = -1;

	ordering return_order; // lasted_how_long has the property that lesser
	// is better, which is opposite what an ordering wants. We fix that by
	// simply inverting the numbers. (Note that we can't do this if we go
	// recursive - but we'll fix that when we get to it).

	for (counter = 0; counter < (size_t)num_candidates; ++counter)
		if (hopefuls[counter])
			return_order.insert(candscore(counter,
					-lasted_how_long[counter] +
					num_hopefuls - 1));

	// If we're only interested in the winner, then we're all done!
	if (winner_only) {
		return (return_order);
	}

	// Determine runs and recurse so we can find out their order.
	// We use a list to store the candidate numbers since we know there
	// can be no ties. At the end we just spool the candidates back into
	// order.

	double oldscore = INFINITY;
	ordering::const_iterator pos = return_order.begin();
	ordering combined_order;
	int norm_score = 0;
	while (pos != return_order.end()) {
		cout << "CHK " << pos->get_score() << ", " << oldscore << endl;
		// If this is true, then we have a run involving the last
		// candidate, this one, and possibly more.
		if (pos->get_score() == oldscore) {
			cout << "------ RECURSING ------" << endl;
			ordering::const_iterator first = pos;
			--first;
			vector<bool> hopefuls_recur(num_candidates, false);
			while (first != return_order.end() && first->get_score()
				== oldscore)
				hopefuls_recur[(first++)->get_candidate_num()]
					= true;

			pos = first;
			ordering subordering = internal_elect(papers, matrix,
					seed, hopefuls_recur, num_candidates,
					winner_only);
			cout << "----- END RECURSION -----" << endl;

			// Incorporate the subordering into the main order.
			// This is kinda ugly and should perhaps be put
			// into a function of its own.

			// TODO?? Use comma here?

			double s_oldscore = INFINITY;
			for (ordering::const_iterator spos = subordering.
					begin(); spos != subordering.end();
				++spos) {
				if (spos->get_score() != s_oldscore || spos
					== subordering.begin()) {
					--norm_score;
				}
				combined_order.insert(candscore(spos->
						get_candidate_num(),
						norm_score));

				s_oldscore = spos->get_score();
			}
			--norm_score;
		} else {
			combined_order.insert(candscore(pos->
					get_candidate_num(),
					norm_score--));
			oldscore = pos->get_score();
			++pos;
		}
	}

	return (combined_order);
}

cond_tournament::cond_tournament(election_method * base_method,
	bool single_round) {
	base = base_method;
	//one_round_only = single_round;
}

pair<ordering, bool> cond_tournament::elect_inner(
	const list<ballot_group> & papers, const ordering & seed,
	const condmat & matrix, bool winner_only) const {

	vector<bool> all_hopeful(matrix.get_num_candidates(), true);

	return (pair<ordering, bool>(internal_elect(papers, matrix, seed,
					all_hopeful,
					matrix.get_num_candidates(),
					winner_only), winner_only));
}

pair<ordering, bool> cond_tournament::elect_inner(
	const list<ballot_group> & papers,
	const vector<bool> & hopefuls, int num_candidates,
	cache_map * cache, bool winner_only) const {

	// Use implied Condorcet matrix with winning-votes. It really
	// doesn't matter whether we use WV or margins, since we only use
	// one bit of information (does A beat B) which is the same for WV
	// and margins.

	ordering seed = base->elect(papers, num_candidates, cache, false);
	condmat matrix(papers, num_candidates, CM_WV);

	return (pair<ordering, bool>(internal_elect(papers, matrix, seed,
					hopefuls, num_candidates, winner_only),
				winner_only));
}

pair<ordering, bool> cond_tournament::elect_inner(
	const list<ballot_group> & papers,
	int num_candidates, cache_map * cache,
	bool winner_only) const {

	vector<bool> all_hopeful(num_candidates, true);

	return (elect_inner(papers, all_hopeful, num_candidates, cache,
				winner_only));
}

string cond_tournament::name() const {
	return ("SE-Condorcet[" + base->name() + "]");
}
