#include "qpq.h"

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

double QPQ::getC(size_t council_size, size_t num_candidates) const {

	if (C_val != -1) {
		return (C_val);
	}

	// So C_val == 1, hence we're using Warren's dynamic scheme.
	// TODO: Source?? Probably the "optimal PR" one.

	// Is this right? What if all vote ... > A > B > C > D > ..
	// then A > B > C > D is a "party" and the numerator here should
	// be decreased accordingly..
	double K = num_candidates/(double)council_size;

	return ((1/K) * log(K/(1-exp(-K))));
}

// It now handles compressed ballots, although they may skew the tiebreak.
std::list<size_t> QPQ::get_council(std::vector<bool> & eliminated,
	size_t council_size, size_t num_candidates, int num_voters,
	const election_t & ballots, bool restart_on_elimination) const {

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

	std::list<size_t> council;
	size_t num_elected = 0, num_eliminated = 0;

	std::vector<bool> elected(num_candidates, false);

	size_t counter;

	double C = getC(council_size, num_candidates);
	assert(C > 0);

	while (num_elected < council_size) {

		// Heuristic that automatically elects rest if
		// just enough to fill council.

		// It's now possible to coerce the thing to use C > 1,
		// but it's not a good idea.

		// If number of candidates still in the running is equal
		// to the number of seats remaining in play, just complete
		// the council.

		if (num_candidates - num_elected - num_eliminated == council_size -
			num_elected) {
			for (size_t i = 0; i < num_candidates; ++i) {
				if (!eliminated[i] && !elected[i]) {
					council.push_back(i);
				}
			}

			return council;
		}

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
			   active_ballots = 0;

		// Bah dual track. Get the active and inactive ballot data.

		for (pos = ballots.begin(); pos != ballots.end(); ++pos) {
			ordering::const_iterator contribute =
				ballot_contribution(eliminated, elected, *pos);

			if (contribute == pos->contents.end()) {
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

		// Test just electing the luckiest (highest) candidate
		// or eliminating the least lucky. Let's see if that helps;
		// if not, I'll implement tiebreak stuff later.

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

			// Could experiment with Condorcet loser elimination
			// here too...
			eliminated[lowest] = true;
			++num_eliminated;

			if (restart_on_elimination) {
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

std::list<size_t> QPQ::get_council(size_t council_size,
	size_t num_candidates,
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
