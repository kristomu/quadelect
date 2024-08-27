#ifndef _VOTE_QPQ
#define _VOTE_QPQ

#include "methods.cc"
//#include "../tiebreaker.cc"
#include <vector>
#include <list>

// NOT YET PORTED.

using namespace std;

class QPQ : public multiwinner_method {
	private:
		ordering::const_iterator ballot_contribution(
			const vector<bool> & eliminated,
			const vector<bool> & elected,
			const ballot_group & ballot) const;

		list<int> get_council(vector<bool> & eliminated,
			int council_size, int num_candidates,
			int num_voters,
			const list<ballot_group> & ballots,
			bool restart_on_elimination) const;

		bool recursive;
		double C_val;	// 1 = D'Hondt, 0.5 = Sainte-Lague,
		// EXPERIMENTAL: -1 = WDS dynamic

		const double getC(int council_size, int num_candidates) const;

	public:
		list<int> get_council(int council_size, int num_candidates,
			const list<ballot_group> & ballots) const;

		string name() const;

		QPQ(double c_in, bool recurs_in) {
			C_val = c_in;
			recursive = recurs_in;
		}
};

ordering::const_iterator QPQ :: ballot_contribution(const vector<bool> &
	eliminated, const vector<bool> & elected,
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

const double QPQ::getC(int council_size, int num_candidates) const {

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
list<int> QPQ::get_council(vector<bool> & eliminated, int council_size,
	int num_candidates, int num_voters,
	const list<ballot_group> & ballots,
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

	vector<double> elect_fraction(num_voters, 0);
	vector<double> contributing_ballots(num_candidates, 0),
		   contributing_weights(num_candidates), quotients(num_candidates);

	list<int> council;
	int num_elected = 0, num_elim = 0;

	vector<bool> elected(num_candidates, false);

	int counter;

	double C = getC(council_size, num_candidates);
	assert(0 <= C && C <= 1);

	list<ordering> past_orderings; // For tie-breaking using the "first
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

		list<ballot_group>::const_iterator pos;
		counter = 0;

		double inactive_ballots = 0, inactive_ballot_fraction = 0,
			   active_ballots = 0, sum_weights = 0;

		// Bah dual track. Get the active and inactive ballot data.

		for (pos = ballots.begin(); pos != ballots.end(); ++pos) {
			ordering::const_iterator contribute =
				ballot_contribution(eliminated, elected, *pos);

			sum_weights += elect_fraction[counter] * pos->weight;

			if (contribute == pos->contents.end()) {
				inactive_ballots += pos->weight;
				inactive_ballot_fraction +=
					elect_fraction[counter] * pos->weight;
			} else {
				active_ballots += pos->weight;
				contributing_ballots[contribute->
					get_candidate_num()] += pos->weight;
				contributing_weights[contribute->
					get_candidate_num()] +=
						elect_fraction[counter] * pos->weight;
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
		set<int> election_candidates, elimination_candidates;
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
			cout << "QPQ: Doing difference tiebreak on electeds."
				<< endl;
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
					cout << "QPQ: Elect. tie remains, breaking first."
						<< endl;
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
			cout << "QPQ: Doing difference tiebreak on elims." <<
				" num_elected: " << num_elected <<
				" remain: " << num_candidates - (num_elected +
					num_elim) << endl;
			elimination_candidates = intersect_orders(
					elimination_candidates, past_orderings,
					false, !first);

			// Same as above
			if (elimination_candidates.size() > 1)
				elimination_candidates = intersect_ballots(
						elimination_candidates,
						ballots.rbegin(),
						ballots.rend(),false);

			if (elimination_candidates.size() > 1) {
				cout << "QPQ: Elim. tie remains, breaking "<<
					"first." << endl;
			}
			lowest = *elimination_candidates.begin();
			cout << "Picked " << lowest << endl;
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
				if (contribute->get_candidate_num() == highest)
					/*elect_fraction[counter] = 1.0/
						quotients[highest];*/
				{
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
				cout << "Recursing with " << num_elected << " elected." << endl;
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

list<int> QPQ::get_council(int council_size, int num_candidates,
	const list<ballot_group> & ballots) const {

	vector<bool> eliminated(num_candidates, false);

	int num_voters = ballots.size();

	return (get_council(eliminated, council_size, num_candidates, num_voters,
				ballots, recursive));
}

string QPQ::name() const {
	string divisor;
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

#endif
