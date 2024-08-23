
#include "meek_stv.h"

// This does the Meek "progressive reweighting" on a single ballot, adding the
// proportion of the ballot that goes to each candidate. It breaks when a
// weight is 1, because then subsequent weights are zero and there's no point.
void MeekSTV::absorb_ballot_meek(const ballot_group & ballot,
	const std::vector<double> & weighting,
	std::vector<double> & candidate_tally, double & excess) const {

	// At any step, a candidate receives weight q * (candidate weight).
	// Then the new q is 1 - this. Q starts at 1.
	// TODO: Handle equal ranks. I think only fractional rank makes any
	// sense. Perhaps if A > B = C > D, then B gets q/2 * w_B and C
	// gets q/2 * w_C and the next Q is 1 - (sum of these)?

	double q = 1;

	for (ordering::const_iterator pos = ballot.contents.begin(); pos !=
		ballot.contents.end() && q != 0; ++pos) {
		double this_wt = q * weighting[pos->get_candidate_num()];

		candidate_tally[pos->get_candidate_num()] += this_wt *
			ballot.get_weight();

		//q *= (1 - weighting[pos->get_candidate_num()]);

		q -= this_wt;
		assert(q >= 0 && q <= 1);
	}

	excess += q;
}

void MeekSTV::absorb_ballot_warren(const ballot_group & ballot,
	const std::vector<double> & weighting,
	std::vector<double> & candidate_tally, double & excess) const {

	// The proportion of the vote apportioned to first preference is
	// equal to the keep value of that first preference.
	// If adding the second preference's keep value would exceed 1,
	// then the second preference gets 1 - (cumulative), and nothing
	// more is apportioned, else
	// the second preference's keep value is apportioned to the second
	// preference.
	// 	If adding the third preference (...) as above.

	double cumulative = 0;

	for (ordering::const_iterator pos = ballot.contents.begin(); pos !=
		ballot.contents.end(); ++pos) {

		double cur_wt = weighting[pos->get_candidate_num()];

		if (cumulative + cur_wt > 1) {
			candidate_tally[pos->get_candidate_num()] += (1 -
					cumulative);
			// Excess is always zero?
			return;
		}

		candidate_tally[pos->get_candidate_num()] += cur_wt;

		cumulative += cur_wt;
	}

	if (cumulative < 1) {
		excess += (1 - cumulative);
	}
}

// This can be turned into a boolean limiter that breaks early, but later.
double MeekSTV::absolute_quota_error(const std::vector<double> &
	cand_tally,
	const std::list<int> & elected, double quota) const {

	// Return the worst discrepancy from quota for those candidates that
	// have already been elected.
	// A later trick could move the worst one to the beginning so that
	// it aborts earlier, but I don't think it's worth the complexity.

	double record = 0;

	for (std::list<int>::const_iterator cand_pos = elected.begin(); cand_pos !=
		elected.end(); ++cand_pos)
		if (fabs(cand_tally[*cand_pos] - quota) > record) {
			record = fabs(cand_tally[*cand_pos] - quota);
		}

	return (record);
}


std::list<int> MeekSTV::get_council(int council_size, int num_candidates,
	const election_t & ballots) const {

	// Algorithm: At each stage, a candidate is either elected, hopeful, or
	// eliminated. Also, all candidates have an internal weighting.
	// Hopeful candidates have internal weight 1, eliminated ones internal
	// weight 0, and elected ones a varying weight determined by the
	// convergence system.

	// We start off by counting the votes, all candidates are hopeful.
	// Any and all candidates whose total votes >= quota become elected,
	// except in the tie situation where all the rest have zero votes,
	// in which case one of the remaining >= quota candidates are removed
	// and the rest is the council.

	// The quota is (#votes - excess) / (seats + 1) - Droop.

	// If nobody meets the quota, eliminate the one with the lowest score.

	// For each iteration, check the votes array and clamp weight values to
	// [0..1]. If there's an elected candidate and he hasn't got within
	// epsilon of a quota, then converge to a new quota. Convergence goes
	// like this:
	// 	For any candidate that's not either eliminated or hopeful,
	// 	the new weight is (old weight) * q / (votes for this candidate)
	// 	where q is the new quota.
	// 	Then count the number of votes with this new weight, and
	// 	calculate a new quota based on the excess returned.
	// That can be done easily with a while loop.


	std::vector<bool> eliminated(num_candidates, false),
		elected(num_candidates,
			false);

	size_t num_elected = 0, counter;
	std::list<int> council;

	std::vector<double> cand_tally(num_candidates);
	std::vector<double> candidate_weight(num_candidates, 1);

	double excess;

	double epsilon = 1e-3; // Accept convergence when we get within this
	// range of a quota.

	double unweighted_sum = 0; // Sum of initial (non-STV) weights.

	election_t::const_iterator pos;

	for (pos = ballots.begin(); pos != ballots.end(); ++pos) {
		unweighted_sum += pos->get_weight();
	}

	// "First difference" approximation. TODO: Fix later, and make changeable
	plurality plur_count(PT_WHOLE);
	ordering tiebreaker = plur_count.elect(ballots,
			num_candidates, NULL, false);
	ordering::const_iterator opos, vpos;

	while (num_elected < (size_t)council_size) {

		// Reset the vote count and excess
		fill(cand_tally.begin(), cand_tally.end(), 0);
		excess = 0;

		// Count the votes.
		for (pos = ballots.begin(); pos != ballots.end(); ++pos) {
			if (warren)
				absorb_ballot_warren(*pos, candidate_weight,
					cand_tally, excess);
			else
				absorb_ballot_meek(*pos, candidate_weight,
					cand_tally, excess);
		}

		// Get quota.
		double quota = (unweighted_sum - excess) / (double)
			(council_size + 1);

		// Readjust the quota if required. When nobody's elected,
		// the absolute quota error return 0 since it has no worse
		// record, and thus the entire convergence loop is skipped.
		double error = absolute_quota_error(cand_tally, council, quota);

		while (error > epsilon) {
			for (std::list<int>::const_iterator cpos = council.begin();
				cpos != council.end(); ++cpos) {

				candidate_weight[*cpos] = (candidate_weight[*cpos] * quota) /
					cand_tally[*cpos];

				// Take care of certain pathological data.
				candidate_weight[*cpos] = std::min(1.0,
						std::max(0.0, candidate_weight[*cpos]));
			}

			// Recount
			excess = 0;
			fill(cand_tally.begin(), cand_tally.end(), 0);

			for (pos = ballots.begin(); pos != ballots.end();
				++pos) {
				if (warren)
					absorb_ballot_warren(*pos,
						candidate_weight,
						cand_tally, excess);
				else
					absorb_ballot_meek(*pos,
						candidate_weight,
						cand_tally, excess);
			}

			// Recalc quota and error.
			quota = (unweighted_sum - excess) / (double)
				(council_size +1);

			error = absolute_quota_error(cand_tally, council,
					quota);
		}

		// Now check for candidates to elect.
		// TODO: First gather all who are above quota into an array,
		// then shuffle and draw up to and including council_size -
		// num_elected more.
		int elected_this_round = 0;

		std::set<int> potential_elected; // This is required to handle the
		// edge case where all + 1 meet the quota exactly. In that
		// case, we'll eliminate the one that fails the tiebreaker.
		// If it's still a tie, the first one (NON-NEUTRAL) goes.

		for (counter = 0; counter < cand_tally.size() ; ++counter) {
			if (eliminated[counter]) {
				continue;
			}

			if (cand_tally[counter] >= quota && !elected[counter]) {
				potential_elected.insert(counter);
			}
		}
		// If it's too large, narrow it down
		if (num_elected + potential_elected.size() > (size_t)council_size) {
			size_t desired_size = council_size - num_elected;

			// CUT AND PASTE TODO

			ordering intersect;
			std::vector<bool> contained(num_candidates, false);
			for (std::set<int>::const_iterator lp = potential_elected.
					begin(); lp != potential_elected.end();
				++lp) {
				contained[*lp] = true;
			}

			for (opos = tiebreaker.begin(); opos != tiebreaker.end();
				++opos)
				if (contained[opos->get_candidate_num()]) {
					intersect.insert(*opos);
				}

			// Now pick off the first desired_size
			std::set<int> desired;
			ordering::const_iterator op = intersect.begin();
			for (counter = 0; counter < desired_size && op !=
				intersect.end(); ++counter) {
				desired.insert((op++)->get_candidate_num());
			}

			potential_elected = desired;
		}

		for (std::set<int>::const_iterator qcpos = potential_elected.begin();
			qcpos != potential_elected.end(); ++qcpos) {

			elected[*qcpos] = true;
			++num_elected;
			++elected_this_round;
			council.push_back(*qcpos);
		}

		// If nobody were elected, someone must be eliminated.
		if (elected_this_round == 0) {
			// Check for whoever had the lowest score, then he's
			// out. TODO: Handle ties. Note that only one candidate
			// can be eliminated in each round, unlike the elected
			// step where multiple may be elected in one round.
			// Also TODO: BTR/STV-ME variants. Instead of eliminating
			// the loser, eliminate the one that pairwise loses to
			// the other; when we have multiple candidates, do either
			// a Condorcet or Plurality check among all and eliminate
			// the loser.
			int recordholder = -1;

			for (counter = 0; counter < cand_tally.size();
				++counter) {
				if (elected[counter] || eliminated[counter]) {
					continue;
				}
				if (recordholder == -1) {
					recordholder = counter;
				}
				if (cand_tally[counter] <=
					cand_tally[recordholder]) {
					recordholder = counter;
				}
			}

			// Fix cut and paste code later!
			std::list<int> tied_for_last;

			for (counter = 0; counter < cand_tally.size();
				++counter) {
				if (elected[counter] || eliminated[counter]) {
					continue;
				}
				if (cand_tally[counter] ==
					cand_tally[recordholder]) {
					tied_for_last.push_back(counter);
				}
			}

			if ((++tied_for_last.begin()) != tied_for_last.end()) {

				std::list<int> recordholders;
				std::vector<bool> contained(num_candidates, false);
				for (std::list<int>::const_iterator lp =
						tied_for_last.begin(); lp !=
					tied_for_last.end(); ++lp) {
					contained[*lp] = true;
				}

				for (opos = tiebreaker.begin(); opos !=
					tiebreaker.end(); ++opos) {
					if (contained[opos->get_candidate_num()])
						recordholders.push_back(
							opos->get_candidate_num());
				}

				recordholder = *(recordholders.rbegin());
			}

			eliminated[recordholder] = true;
			candidate_weight[recordholder] = 0;
		}
	}

	return (council);
}