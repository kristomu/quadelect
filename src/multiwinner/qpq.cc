#include "qpq.h"

/* TODO: REMOVE THIS. Once we have a figure for QPQ. */

std::set<size_t> get_tied_candidates(const ordering & social_order,
	double tied_score) {

	candscore bound_score(0, tied_score);

	// Ugly linear search hack. Fix later!

	std::set<size_t> candidates;

	for (ordering::const_iterator pos = social_order.begin(); pos !=
		social_order.end(); ++pos)
		if (pos->get_score() == tied_score) {
			candidates.insert(pos->get_candidate_num());
		}

	return (candidates);
}

std::set<size_t> intersect_order(const std::set<size_t> & input,
	const ordering & social_order, bool best) {

	// First find the candidate that's best (resp worst) ranked. That
	// takes lg n (find) times n (number of candidates) time.

	//cout << "RS: " << social_order.size() << endl;

	ordering::const_iterator record_cand = social_order.end();
	bool new_record = true; // so it's immediately set.

	assert(!input.empty());

	for (std::set<size_t>::const_iterator p = input.begin(); p != input.end();
		++p) {
		// This has now turned into a linear search because the < and >
		// on candscore isn't transitive. Fix later!! Or make a map
		// or something.
		ordering::const_iterator matching_cand = social_order.end(), q;
		for (q = social_order.begin(); q != social_order.end() &&
			matching_cand == social_order.end(); ++q)
			if (q->get_candidate_num() == *p) {
				matching_cand = q;
			}

		// DEBUG
		/*cout << "Looking for " << *p << endl;
		for (q = social_order.begin(); q != social_order.end(); ++q) {
			cout << "( " << q->get_candidate_num() << ", " <<
				q->get_score() << ")" << endl;
		}*/

		/*ordering::const_iterator matching_cand = social_order.lower_bound(
				candscore(*p, 100));*/
		assert(matching_cand != social_order.end());
		//assert (*matching_cand == *p);

		if (!new_record) {
			if (best)
				new_record = matching_cand->get_score() >
					record_cand->get_score();
			else	new_record = matching_cand->get_score() <
					record_cand->get_score();
		}

		if (new_record) {
			//cout << "acc new record " << matching_cand->get_candidate_num() << endl;
			record_cand = matching_cand;
			new_record = false;
		}
		//cout << "next" << endl;
	}

	assert(record_cand != social_order.end());

	// Then get a tied_candidate set for the score of whoever was best
	// (or worst) ranked. That takes lg n time.
	std::set<size_t> tied_for_preferrable = get_tied_candidates(social_order,
			record_cand->get_score());

	// Then do an intersection of the two sets. That takes linear time.

	std::set<size_t> intersect;
	set_intersection(input.begin(), input.end(),
		tied_for_preferrable.begin(),
		tied_for_preferrable.end(), inserter(
			intersect, intersect.begin()));

	/*cout << "DEBUG: tfp: ";
	copy(tied_for_preferrable.begin(), tied_for_preferrable.end(),
			ostream_iterator<int>(cout, " "));
	cout << endl << "DBUG: input: ";
	copy(input.begin(), input.end(), ostream_iterator<int>(cout, " "));

	cout << endl;*/

	// Finally, return.

	return (intersect);
}

template<typename A> std::set<size_t> intersect_orders(
	const std::set<size_t> & input,
	const A & iter_start, const A & iter_end, bool best) {

	std::set<size_t> shaved = input;

	for (A pos = iter_start; pos != iter_end && shaved.size() != 1; ++pos) {
		shaved = intersect_order(shaved, *pos, best);
	}

	return (shaved);
}

template<typename A> std::set<size_t> intersect_ballots(
	std::set<size_t> process,
	const A & iter_start, const A & iter_end, bool best) {

	for (A pos = iter_start; pos != iter_end && process.size() != 1; ++pos) {
		process = intersect_order(process, pos->contents, best);
	}

	return (process);
}

std::set<size_t> intersect_orders(const std::set<size_t> & input,
	const std::list<ordering> & social_orders, bool best,
	bool reversed) {

	if (reversed) {
		return (intersect_orders(input, social_orders.rbegin(),
					social_orders.rend(), best));
	} else {
		return (intersect_orders(input, social_orders.begin(),
					social_orders.end(), best));
	}
}

/** REMOVAL END **/

ordering::const_iterator QPQ::ballot_contribution(const std::vector<bool> &
	eliminated, const std::vector<bool> & elected,
	const ballot_group & ballot) const {

	// Find out which candidate this ballot "contributes" to - i.e the
	// first non-elected, non-eliminated candidate. If there is none
	// and the ballot is "inactive", then return ballot.contents.end().

	for (ordering::const_iterator opos = ballot.contents.begin();
		opos != ballot.contents.end(); ++opos)
		if (!eliminated[opos->get_candidate_num()] && !elected[
				opos->get_candidate_num()]) {
			return (opos);
		}

	return (ballot.contents.end());
}

double QPQ::getC(int council_size, int num_candidates) const {

	if (C_val != -1) {
		return (C_val);
	}

	// Is this right? What if all vote ... > A > B > C > D > ..
	// then A > B > C > D is a "party" and the numerator here should
	// be decreased accordingly..
	double K = num_candidates/(double)council_size;

	return ((1/K) * log(K/(1-exp(-K))));
}

// It now handles compressed ballots, although they may skew the tiebreak.
std::list<int> QPQ::get_council(std::vector<bool> & eliminated,
	int council_size,
	int num_candidates, int num_voters,
	const election_t & ballots,
	bool restart_on_elimination) const {

	// Algorithm:
	// The method proceeds in stages. At each stage, each ballot is said to
	// have elected a fractional number of candidates with the constraint
	// that the sum of these over all the ballots equal the number of
	// candidates that are currently elected. We begin with each ballot
	// election fraction being zero.
	//
	// At the start of a stage, calculate each candidate's quotient unless
	// that candidate is either elected or eliminated. A candidate c's
	// quotient is equal to v_c / (1 + t_c) where v_c is the number of
	// "contributing" ballots (see previous function) and t_c is the sum
	// of the fractional candidates those contributing ballots have
	// elected.
	//
	// Any ballot that contributes to some candidate is active.
	// After calculating quotients, calculate the quota q = v_a/(1+s - t_x)
	// where v_a is the number of active ballots, s is the council size,
	// and t_x is the sum of the fractional candidates elected by inactive
	// ballots.
	//
	// If some candidate has a quotient that exceeds the quota, then elect
	// the highest-quotient candidate. Set the election fraction of to 1/q_c
	// for all those ballots that contributed to the candidate in question.
	// If we now have the council, return it.
	//
	// On the other hand, if no candidate exceeds the quota, then whoever
	// has the smallest quotient is excluded. (Break ties randomly or by
	// highest earlier rank). If there are exactly enough candidates left
	// to fill the council, elect them and we're done. Otherwise, proceed
	// to the next stage or recurse, depending on whether restart_on_elim
	// is false or true.

	std::vector<double> elect_fraction(num_voters, 0);
	std::vector<double> contributing_ballots(num_candidates, 0),
		contributing_weights(num_candidates), quotients(num_candidates);

	std::list<int> council;
	int num_elected = 0, num_elim = 0;

	std::vector<bool> elected(num_candidates, false);

	int counter;

	double C = getC(council_size, num_candidates);
	assert(0 <= C && C <= 1);

	std::list<ordering> past_orderings; // For tie-breaking using the "first
	// difference" rule. TODO: Also try "last difference" or externally
	// supplied ordering from Condorcet/whatever (unweighted, or weighted
	// according to quotients).

	while (num_elected < council_size) {
		fill(contributing_ballots.begin(), contributing_ballots.end(),
			0);
		fill(contributing_weights.begin(), contributing_weights.end(),
			0);

		// Find out who the various ballots contribute to, and also
		// count the number of inactive ballots (and the combined
		// fraction of these).

		election_t::const_iterator pos;
		counter = 0;

		double inactive_ballot_fraction = 0,
			   active_ballots = 0, sum_weights = 0;

		// Bah dual track. Get the active and inactive ballot data.

		for (pos = ballots.begin(); pos != ballots.end(); ++pos) {
			ordering::const_iterator contribute =
				ballot_contribution(eliminated, elected, *pos);

			sum_weights += elect_fraction[counter] * pos->get_weight();

			if (contribute == pos->contents.end()) {
				//inactive_ballots += pos->get_weight();
				inactive_ballot_fraction +=
					elect_fraction[counter] * pos->get_weight();
			} else {
				active_ballots += pos->get_weight();
				contributing_ballots[contribute->
					get_candidate_num()] += pos->get_weight();
				contributing_weights[contribute->
					get_candidate_num()] +=
						elect_fraction[counter] * pos->get_weight();
			}
			++counter;
		}

		// Quick and dirty way of getting rid of the problems with
		// precision mismatch. It should still sum to the number
		// elected, but this way we won't break the assert if there's
		// a spurious 1e-16 error or something.
		// DONE: Fix really strange bug here.
		assert(round(sum_weights) == num_elected);

		// Calculate quotients.
		for (counter = 0; counter < num_candidates; ++counter)
			quotients[counter] = (C * contributing_ballots[counter])
				/ (C + contributing_weights[counter]);

		// And calculate quota.
		double quota = (C * active_ballots) / (C + council_size -
				inactive_ballot_fraction);

		// Determine lowest and highest scoring candidates, breaking
		// ties "randomly" (last candidate wins). TODO: More
		// sophisticated tiebreaking so that it's truly neutral.
		int lowest = -1, highest = -1;

		for (counter = 0; counter < num_candidates; ++counter)
			if (!eliminated[counter] && !elected[counter]) {
				if (lowest == -1)	{
					lowest = counter;
				}
				if (highest == -1)	{
					highest = counter;
				}

				if (quotients[counter] <= quotients[lowest]) {
					lowest = counter;
				}
				if (quotients[counter] >= quotients[highest]) {
					highest = counter;
				}
			}

		assert(lowest != -1 && highest != -1);

		// Sophisticated tiebreaking: Okay, now we know the low and
		// high records. Dump all who are above quota and also equal
		// to the high record into election_candidates, and those
		// that are lowest into elimination_candidates. Most of the
		// time, these will be unit sets.

		// Note: This might be a memory leak. Check later.
		std::set<size_t> election_candidates, elimination_candidates;
		ordering current_ordering;

		for (counter = 0; counter < num_candidates; ++counter) {
			if (eliminated[counter] || elected[counter]) {
				continue;
			}
			if (quotients[counter] >= quotients[highest] &&
				quotients[counter] > quota) {
				election_candidates.insert(counter);
			}

			if (quotients[counter] <= quotients[lowest]) {
				elimination_candidates.insert(counter);
			}

			current_ordering.insert(candscore(counter,
					quotients[counter]));
		}

		past_orderings.push_back(current_ordering);

		// If any of the sets are non-unit and not empty, then break
		// ties by first-difference. Tell the user this (DEBUG).
		// Perhaps tri-valued: -1 means first difference, 1 last
		// difference, 0 proceed immediately to ultimate tiebreaker.

		bool first = true; // first = false gives slightly worse
		// results.

		if (election_candidates.size() > 1) {
			std::cout << "QPQ: Doing difference tiebreak on electeds."
				<< std::endl;
			election_candidates = intersect_orders(
					election_candidates, past_orderings,
					true, !first);
			// If there are still ties, break first and tell
			// the user, otherwise pick the one remaining.
			// NOTE: Not cloneproof nor neutral!
			// TODO: Random ballot or "random voter hierarchy". RVH
			// is just intersect_orders with a random permutation
			// of the entire ballot list until either all are
			// exhausted or we have a single candidate.
			// Also TODO: Case that automatically breaks the ties
			// the same way when recursing. Perhaps a rule like
			// "if there's a tie, and one of the candidates tied
			//  were eliminated earlier, he's the one who gets
			//  eliminated".. Nah, that won't work.
			if (election_candidates.size() > 1) {
				// Last ballot: hackup version of random
				// dictator. RVH actually worsens the result
				// according to the simulation, but we need
				// it for neutrality.
				election_candidates = intersect_ballots(
						election_candidates,
						ballots.rbegin(),
						ballots.rend(), true);
				if (election_candidates.size() > 1) {
					std::cout << "QPQ: Elect. tie remains, breaking first."
						<< std::endl;
				}
				highest = *election_candidates.begin();
			} else {
				highest = *election_candidates.begin();
			}
		}

		// No need to bother with the elimination ones if we
		// already have someone above quota, since we're not going
		// to use it anyhow.
		if (elimination_candidates.size() > 1 &&
			election_candidates.size() == 0) {
			std::cout << "QPQ: Doing difference tiebreak on elims." <<
				" num_elected: " << num_elected <<
				" remain: " << num_candidates - (num_elected +
					num_elim) << std::endl;
			elimination_candidates = intersect_orders(
					elimination_candidates, past_orderings,
					false, !first);

			// Same as above
			if (elimination_candidates.size() > 1) {
				elimination_candidates = intersect_ballots(
						elimination_candidates,
						ballots.rbegin(),
						ballots.rend(),false);
			}

			if (elimination_candidates.size() > 1) {
				std::cout << "QPQ: Elim. tie remains, breaking "<<
					"first." << std::endl;
			}
			lowest = *elimination_candidates.begin();
			std::cout << "Picked " << lowest << std::endl;
		}

		// If there are still ties, break randomly and tell the user.

		// If whoever had the highest quotient meets the quota, then
		// go to election, otherwise go to elimination.
		if (quotients[highest] >= quota) {
			// Readjust the ballots that contributed to this guy
			counter = 0;
			for (pos = ballots.begin(); pos != ballots.end();
				++pos) {
				ordering::const_iterator contribute =
					ballot_contribution(eliminated,
						elected, *pos);
				// XXX: Really should use another sentinel mechanism and
				// have highest/lowest be size_t
				if (contribute->get_candidate_num() == (size_t)highest) {
					elect_fraction[counter] = (1 + contributing_weights[highest]) /
						contributing_ballots[highest];
				}
				++counter;
			}
			// And mark as elected.
			elected[highest] = true;
			council.push_back(highest);
			++num_elected;
		} else {
			// Eliminate. First mark as eliminated. Then if we have
			// reset set to true, tail-recurse. Otherwise, just loop
			// through.
			eliminated[lowest] = true;
			++num_elim;

			// TODO: Heuristic that automatically elects rest if
			// just enough to fill council.

			if (restart_on_elimination) {
				std::cout << "Recursing with " << num_elected << " elected." << std::endl;
				past_orderings.clear();
				return (get_council(eliminated, council_size,
							num_candidates,
							num_voters,
							ballots,
							restart_on_elimination)
					);
			}
		}
	}

	return (council);
}

std::list<int> QPQ::get_council(int council_size, int num_candidates,
	const election_t & ballots) const {

	std::vector<bool> eliminated(num_candidates, false);

	int num_voters = ballots.size();

	return (get_council(eliminated, council_size, num_candidates, num_voters,
				ballots, recursive));
}

std::string QPQ::name() const {
	std::string divisor;
	if (C_val == 1) {
		divisor = "D'Hondt";
	}
	if (C_val == 0.5) {
		divisor = "Sainte-L";
	}
	if (C_val == -1) {
		divisor = "WDS-Dyn";
	}
	if (divisor == "") {
		divisor = dtos(C_val);
	}

	if (recursive) {
		return ("QPQ(div " + divisor + ", multiround)");
	} else {
		return ("QPQ(div " + divisor +", sequential)");
	}
}