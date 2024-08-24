#include "stv.h"

// There's a known bug: this may crash due to hopefuls being all false.
// Find out what's going on and then fix, later.

std::vector<bool> STV::get_btr_stv_hopefuls(const ordering & count,
	const std::vector<bool> & uneliminated, int num_candidates,
	int num_elected, int council_size) const {

	// Determine those that get to run in the "Condorcet loser contest" of
	// BTR-STV. See the main procedure for information on BTR-STV.

	int N = (council_size - num_elected) + 1;

	//cout << "N is " << N << endl;

	std::vector<bool> hopefuls(num_candidates, false);

	ordering::const_reverse_iterator cpos = count.rbegin();

	// The reason for the nested loops here is to handle ties. Multiple
	// tied candidates count as a single "N" candidate. Alternate way
	// would be to break the tie earlier; we may check that later, and it
	// can be done by just passing a tie-broken count in so that the inner
	// loop is always 1.

	while (cpos != count.rend() && N > 0) {
		if (!uneliminated[cpos->get_candidate_num()]) {
			++cpos;
			continue;
		}

		double score_now = cpos->get_score();

		while (cpos->get_score() == score_now && cpos != count.rend()) {
			//		cout << "Setting " << cpos->get_candidate_num() << endl;
			hopefuls[cpos->get_candidate_num()] = uneliminated[
					cpos->get_candidate_num()];
			++cpos;
		}

		--N;
	}

	return (hopefuls);
}

// STV with Gregory/Senatorial rules.
// There may be bugs here. Check later.
// If eliminate is false, when deciding what candidate to remove from
// consideration, we use the original (all candidates included) Plurality
// ranking. Otherwise, we eliminate as in STV.
std::list<int> STV::get_council(int council_size, int num_candidates,
	const election_t & ballots) const {

	// STV rules (Senatorial rules, as they're the easiest to compute):
	// 	While we have candidates remaining:
	// 		1. Do a tally, disregarding those already elected
	// 			or eliminated.
	// 		If someone's above quota
	// 			Elect them.
	// 			Reweight. Say the quota is Q and the candidate
	// 				elected had R > Q votes. All voters now
	// 				get weighted by (R-Q)/Q (since all
	// 				ballots are complete, exhausted is 0).
	// 				(no: It's (R-Q/R. )
	// 			Go to 1.
	// 		Else
	//			Eliminate whoever has the fewest first place
	//			votes.
	//	Loop

	// BTR-STV option:
	// "The elimination round in BTR-STV is to take the N + 1 (where N is
	//  the number of seats that STILL need to be elected in the election)
	//  candidates that have the least votes, and then compare them by
	//  having each voter attach themselves to the candidate of the least
	//  whom they rank highest."
	//
	// This may not be very good, since I think cases exist where the
	// Condorcet loser should be part of the council, but WTH, let's test.
	//
	// Our implementation is like this: Run a Condorcet method on the
	// subset that includes only the N+1 lowest ranked. It's slow, but that
	// way we don't have to care about Condorcet paradoxes, etc.

	std::vector<bool> elected(num_candidates, false),
		eliminated(num_candidates, false), hopefuls(num_candidates,
			true);

	int council_count = 0, num_hopefuls = num_candidates;

	// TODO: Make this changeable
	plurality plur_count(PT_WHOLE);

	// Determine number of voters (sum of weights actually).
	double num_voters = 0;
	election_t::const_iterator lpos;
	election_t::iterator bpos;
	for (lpos = ballots.begin(); lpos != ballots.end(); ++lpos) {
		num_voters += lpos->get_weight();
	}

	// Droop quota. Hare is better but more vulnerable to vote mgmt.
	// TODO: Make this tunable too.
	//double quota = floor(( num_voters / (double)(council_size + 1) ) + 1);

	// Epsilon here? We should also move all of this to a quota class.
	const double quota = num_voters / (double)(council_size + 1);

	election_t reweighted_ballots = ballots;
	std::list<int> council;

	// This is used for tie-breaking as a first-order approximation to
	// the "first-difference" rule in Newland-Britton. We don't actually
	// calculate it until we would have to anyway (that is, the first
	// plurality calc.)
	ordering tiebreaker;
	bool tiebreaker_working = false;

	while (council_count < council_size) {
		assert(num_hopefuls > 0);

		// Do a (plurality) tally, disregarding those who've been
		// eliminated.

		ordering social_order = plur_count.elect(reweighted_ballots,
				hopefuls, num_candidates);

		if (!tiebreaker_working) {
			tiebreaker = social_order;
			tiebreaker_working = true;
		}

		// Check if anyone's above quota. In the case that more are,
		// we should only concern ourselves with the one who got the
		// most votes. Therefore, we handle this by making two lists:
		// one of all that's tied first, and one of all those that are
		// tied last. In most cases, these lists will be of size 1
		// and we can go on our way. If not, break the tie by the
		// tiebreaker ordering, and if that still fails, just pick the
		// first one.

		candscore last_cs = *first_hopeful(social_order.rbegin(),
				social_order.rend(), hopefuls),
			first_cs = *first_hopeful(social_order.begin(),
					social_order.end(), hopefuls);

		std::list<candscore> tied_for_last, tied_for_first;

		ordering::iterator opos = social_order.begin(), vpos;
		for (opos = social_order.begin(); opos != social_order.end();
			++opos) {

			if (opos->get_score() == last_cs.get_score()) {
				tied_for_last.push_back(*opos);
			}
			if (opos->get_score() == first_cs.get_score()) {
				tied_for_first.push_back(*opos);
			}
		}

		// If tied for first has more than one member, break a tie;
		// first according to the tiebreaker, then randomly if that
		// didn't work. We use this odd way of testing for size == 1
		// because size on a linked list is linear.

		candscore top(0), bottom(0);

		if ((++tied_for_first.begin()) != tied_for_first.end()) {
			ordering intersect;
			std::vector<bool> contained(num_candidates, false);
			std::vector<double> relevant_score(num_candidates, 0);
			for (std::list<candscore>::const_iterator lp =
					tied_for_first.begin(); lp !=
				tied_for_first.end(); ++lp) {
				contained[lp->get_candidate_num()] = true;
				relevant_score[lp->get_candidate_num()] =
					lp->get_score();
			}

			for (opos = tiebreaker.begin(); opos != tiebreaker.
				end(); ++opos)
				if (contained[opos->get_candidate_num()]) {
					intersect.insert(*opos);
				}

			// Should be random, but this'll do for testing.
			int candnum = intersect.begin()->get_candidate_num();
			top = candscore(candnum, relevant_score[candnum]);
		} else {
			top = *(tied_for_first.begin());
		}

		if ((++tied_for_last.begin()) != tied_for_last.end()) {
			ordering intersect;
			std::vector<bool> contained(num_candidates, false);
			std::vector<double> relevant_score(num_candidates, 0);
			for (std::list<candscore>::const_iterator lp =
					tied_for_last.begin(); lp !=
				tied_for_last.end(); ++lp) {
				contained[lp->get_candidate_num()] = true;
				relevant_score[lp->get_candidate_num()] =
					lp->get_score();
			}

			for (opos = tiebreaker.begin(); opos != tiebreaker.
				end(); ++opos)
				if (contained[opos->get_candidate_num()]) {
					intersect.insert(*opos);
				}

			int candnum = intersect.rbegin()->get_candidate_num();
			bottom = candscore(candnum, relevant_score[candnum]);
		} else {
			bottom = *(tied_for_last.rbegin());
		}

		// If the top-ranked candidate is above quota, elect him and
		// reweight those who voted for him proportionally to surplus/
		// total vote gathered. Otherwise, eliminate the bottom-ranked
		// candidate.

		if (top.get_score() >= quota) {
			int electable = top.get_candidate_num();
			double score = top.get_score();
			double surplus = score - quota;

			// Reweight those that voted for him.
			// We can make this part sublinear but that'd
			// break our abstraction.
			// The somewhat unwieldy while() construct here
			// is due to erase() returning an iterator *after*
			// the one that was erased.

			bpos = reweighted_ballots.begin();
			while (bpos != reweighted_ballots.end()) {
				if (plur_count.get_pos_score(*bpos,
						electable, hopefuls, num_hopefuls) <= 0) {

					++bpos;
					continue;
				}

				double before = bpos->get_weight();
				if (!isfinite(before)) {
					throw std::invalid_argument(
						"STV: can't handle ballot with infinite weight");
				}

				if (surplus == 0) {
					bpos = reweighted_ballots.erase(bpos);
				} else {
					bpos->set_weight(before * surplus / score);
					++bpos;
				}
			}

			elected[electable] = true;
			hopefuls[electable] = false;
			council.push_back(electable);
			++council_count;
			--num_hopefuls;
		} else {
			int last_candidate;

			// Eliminate whoever came in last. TODO: Add support
			// for Borda-STV (eliminates in Borda order no matter
			// who got last number of first place votes) and,
			// generalized, any sort of Condorcetian completion.
			// Even more exotic would be the elimination of the
			// current loser based on reweighted ballots.

			// The entire chunk here should be moved to a subclass
			// so that we may BTR-ify Meek and Warren.
			if (btr_stv != BTR_NONE) {
				// TODO: Proper tiebreak if multiple candidates
				// end up last.
				// TODO: Extensible so we can have condorcet,
				// Plurality.
				ordering elim_check;
				std::vector<bool> purgatory = get_btr_stv_hopefuls(
						social_order, hopefuls, num_candidates,
						council_count, council_size);

				if (btr_stv == BTR_COND) {
					condmat condorcet(ballots,
						num_candidates, CM_WV);

					// was minmax
					elim_check = schulze(CM_WV).pair_elect(
							condorcet, purgatory, false).first;
				} else {
					elim_check = plur_count.elect(ballots,
							purgatory, num_candidates);
				}

				ordering::iterator f = elim_check.begin();

				ordering filter_elimcheck;

				for (f = elim_check.begin(); f !=
					elim_check.end(); ++f)
					if (purgatory[f->get_candidate_num()]) {
						filter_elimcheck.insert(*f);
					}

				elim_check = filter_elimcheck;

				// Beware random tiebreaking! TODO: Fix.
				// Clean up the ugliness above, too.

				last_candidate = elim_check.rbegin()->get_candidate_num();
				assert(purgatory[last_candidate]);
			} else {
				last_candidate = bottom.get_candidate_num();
			}

			assert(hopefuls[last_candidate]);

			eliminated[last_candidate] = true;
			hopefuls[last_candidate] = false;
			--num_hopefuls;
		}
	}

	return (council);

}