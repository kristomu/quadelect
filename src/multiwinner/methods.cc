
#include <numeric>
#include <algorithm>
#include <iterator>
#include <vector>
#include <list>
#include <set>

#include "methods.h"

// ------------------ Majoritarian --------------

std::list<int> majoritarian_council::get_council(int council_size,
	int num_candidates, const election_t & ballots) const {

	// First get the ordering

	ordering social_order = base->elect(ballots,
			num_candidates, NULL, false);

	// Then pick the first n (note that this, strictly speaking, fails
	// neutrality. Fix later) and return it.

	// Possible way of fixing neutrality. For each level (equal rank set),
	// acquire those that are ranked at that level, shuffle the result,
	// and read off until the council has been filled.

	std::list<int> council;

	int count = 0;

	for (ordering::const_iterator pos = social_order.begin(); pos !=
		social_order.end() && count < council_size; ++pos) {
		council.push_back(pos->get_candidate_num());
		++count;
	}

	return (council);
}

// ------------------ Random --------------

std::list<int> random_council::get_council(int council_size,
	int num_candidates,
	const election_t & ballots) const {

	// First generate a candidate list 0..num_candidates. Then shuffle
	// them randomly and pick the first council_size number.

	// This could be cached.
	std::vector<int> numbered_candidates(num_candidates);
	std::iota(numbered_candidates.begin(), numbered_candidates.end(), 0);

	std::random_shuffle(numbered_candidates.begin(),
		numbered_candidates.end());

	std::list<int> toRet;

	std::copy(numbered_candidates.begin(), numbered_candidates.begin() +
		council_size, inserter(toRet, toRet.begin()));

	return toRet;
}

// ---------------- Multiplicative voter-based reweighting. ------------

std::list<int> mult_ballot_reweighting::get_council(int council_size,
	int num_candidates, const election_t & ballots) const {

	// first get the social ordering for the entire ballot set. Once that's
	// done, pick the unelected candidate that's closest to the winner (and
	// may often be the winner outright). Then, for each voter, get that
	// candidate's score based on the voter's ballot alone, and reweight by
	// C / (C + relative power), where relative power is how many points he
	// contributed, scaled so that the maximum number of points is one and
	// the minimum is zero.

	// Since determining the social order is relatively expensive for a
	// positional system when candidates >> voters, we need a separate
	// function or heuristic for this so that it just calculates the score
	// directly in that case. (We do the former, should do the latter,
	// though.)

	// TODO: Somehow unify this with cardinal ratings, where the voter's
	// relative power is just his score relative tot he maximum. And
	// cumulative vote as well.

	std::list<int> council;
	int count = 0;

	election_t reweighted_ballots = ballots;

	std::vector<bool> elected(num_candidates, false);
	std::vector<bool> hopefuls(num_candidates, true);

	while (count < council_size) {

		ordering social_order = base->elect(reweighted_ballots,
				num_candidates, NULL, false);

		// Get the winner.
		int next = -1;
		for (ordering::const_iterator pos = social_order.begin(); pos !=
			social_order.end() && next == -1; ++pos)
			if (!elected[pos->get_candidate_num()]) {
				next = pos->get_candidate_num();
			}

		assert(next != -1);

		// Elect him.
		council.push_back(next);
		elected[next] = true;
		++count;

		if (count == council_size) {
			continue;
		}

		// Reweight the voters according to their contribution to
		// the person's victory. (Perhaps a more advanced mechanism
		// would use quota and surplus instead, in two steps).
		// 	But that'd require elimination, at which point we have
		// 	STV.

		// Should this be "num_unelecteds" ?
		double maxscore = base->get_pos_maximum(num_candidates),
			   minscore = base->get_pos_minimum(num_candidates);

		// Perhaps throw an exception here instead? If the positional
		// method gives every candidate the same amount of points,
		// there's really nothing we can do.
		if (maxscore - minscore == 0) {
			maxscore = minscore + 1e-6;
		}

		// TODO: Handle fractional votes, since they only count
		// (value / number of equally ranked votes) and so gps should
		// be divided by (number of equally ranked votes) in that case.
		for (election_t::iterator ballot_pos =
				reweighted_ballots.begin(); ballot_pos !=
			reweighted_ballots.end(); ++ballot_pos) {

			double current_state = base->get_pos_score(
					*ballot_pos, next, hopefuls, num_candidates);
			double relative_power = renorm(minscore, maxscore,
					current_state, 0.0, 1.0);
			double inner_denom = B + relative_power;

			if (inner_denom == 0) {
				continue;
			}
			assert(inner_denom > 0);

			//cout << "Debug " << name() << ": " << current_state << "\t" << inner_denom << "\t" << C / inner_denom << "\t" << ballot_pos->get_weight() << " -> ";

			ballot_pos->set_weight(ballot_pos->get_weight() * A /
				inner_denom);
			//cout << ballot_pos->get_weight() << endl;
		}
	}

	return council;
}

// --------------------- Additive voter-base reweighting. ---------------------

std::list<int> addt_ballot_reweighting::get_council(int council_size,
	int num_candidates, const election_t & ballots) const {

	// First get the original weightings and count the number of ballots.

	std::vector<double> original_weights;

	election_t::const_iterator qpos;
	election_t::iterator wpos;

	for (qpos = ballots.begin(); qpos != ballots.end(); ++qpos) {
		original_weights.push_back(qpos->get_weight());
	}

	size_t num_ballots = original_weights.size();

	std::vector<double> total_power_given(num_ballots, 0);

	std::list<int> council;
	int council_count = 0;

	std::vector<bool> elected(num_candidates, 0);
	std::vector<bool> hopefuls(num_candidates, true);

	election_t reweighted_ballots = ballots;

	while (council_count < council_size) {
		// Calculate who wins this round.

		ordering social_order = base->elect(reweighted_ballots,
				num_candidates, NULL, false);

		int next = -1;
		for (ordering::const_iterator opos = social_order.begin();
			opos != social_order.end() && next == -1;
			++opos)
			if (!elected[opos->get_candidate_num()]) {
				next = opos->get_candidate_num();
			}

		// Set this guy to elected, and update the weight counts

		assert(next != -1);

		elected[next] = true;
		council.push_back(next);
		++council_count;
		if (council_count == council_size) {
			continue;
		}

		double maxscore = base->get_pos_maximum(num_candidates - council_count),
			   minscore = base->get_pos_minimum(num_candidates - council_count);

		size_t ballot_idx = 0;

		for (wpos = reweighted_ballots.begin(); wpos !=
			reweighted_ballots.end(); ++wpos) {

			double points_given = base->get_pos_score(
					*wpos, next, hopefuls, num_candidates);

			double relative_power = renorm(minscore, maxscore,
					points_given, 0.0, 1.0);

			total_power_given[ballot_idx] += relative_power;

			double C = 0.5;

			// This generalizes Sainte-Laguë because 0.5 / (0.5 + k) =
			// 1/(2k) for integer k. The total power it's possible to give
			// is if the voter provides maximum voting power to the elected
			// candidate in every round, thus total power given would be the
			// number of rounds, as desired.

			wpos->set_weight(original_weights[ballot_idx] * C / (C +
					total_power_given[ballot_idx]));
		}
	}

	return (council);
}
