#include "auction.h"


int r_auction::elect_and_update(std::vector<bool> & elected,
	std::vector<double> & account_balance,
	const election_t & ballots, double minimum,
	double maximum, size_t num_candidates, bool cumulative) const {

	// Some sanity checks.
	assert(num_candidates > 0);
	assert(minimum <= maximum);

	// Create a sum array for the candidates.
	std::vector<double> sum(num_candidates, 0);

	// For each voter, minimum corresponds to zero "dollars", whereas
	// maximum corresponds to account_balance. Add the rescaled ballot
	// to the sum (for all non-elected candidates). Note the potential
	// for vote-management.
	size_t counter = 0;

	for (election_t::const_iterator pos = ballots.begin(); pos !=
		ballots.end(); ++pos) {
		assert(pos->rated);

		double adjust = 0; // maximum if not cumulative, sum if so.
		ordering::const_iterator sec;

		if (cumulative) {
			for (ordering::const_iterator sec = pos->contents.
					begin(); sec != pos->contents.end();
				++sec) {
				adjust += sec->get_score();
			}
		} else {
			adjust = maximum;
		}

		for (ordering::const_iterator sec = pos->contents.begin();
			sec != pos->contents.end(); ++sec) {
			assert(sec->get_candidate_num() <
				num_candidates);

			if (elected[sec->get_candidate_num()]) {
				continue;
			}
			double norm_score = renorm(minimum, adjust,
					sec->get_score(), 0.0,
					account_balance[counter]);
			sum[sec->get_candidate_num()] += norm_score *
				pos->get_weight();
		}
		++counter;
	}

	// Determine the winner and the second place. Tiebreak is non-neutral!

	size_t record_pos = 0;
	for (counter = 1; counter < (size_t)num_candidates; ++counter)
		if (sum[counter] > sum[record_pos] && !elected[counter]) {
			record_pos = counter;
		}

	size_t second_place = 0;
	bool set_second_place = false;
	for (counter = 0; counter < (size_t)num_candidates; ++counter) {
		if (counter == record_pos) {
			continue;    // first place
		}
		if (!set_second_place) {
			second_place = counter;
			set_second_place = true;
			continue;
		}
		if (sum[counter] > sum[second_place] && !elected[counter]) {
			second_place = counter;
		}
	}

	// Sanity check on output. Fails if num_candidates is 1.
	assert(second_place != record_pos && set_second_place);

	// Elect whoever came in first.
	elected[record_pos] = true;

	// Adjust the voters' accounts.
	double diminuation = sum[second_place]/sum[record_pos];

	// For each voter, determine his adjusted rating of the winner. That
	// candidate loses money equal to the rating, times "diminuation".
	counter = 0;
	for (election_t::const_iterator pos = ballots.begin(); pos !=
		ballots.end(); ++pos) {
		bool rated_winner = false;
		double winner_rating;

		for (ordering::const_iterator sec = pos->contents.begin();
			sec != pos->contents.end() && !rated_winner;
			++sec) {
			if (sec->get_candidate_num() == record_pos) {
				rated_winner = true;
				winner_rating = renorm(minimum, maximum,
						sec->get_score(), 0.0,
						account_balance[counter]);
			}
		}

		if (rated_winner)
			account_balance[counter] -= winner_rating *
				diminuation;

		++counter;
	}

	return (record_pos); // next winner
}

council_t r_auction::get_council(size_t council_size,
	size_t num_candidates,
	const election_t & ballots) const {

	std::vector<bool> elected(num_candidates, false);
	// Denomination makes no difference, so let's have the standard weight
	// as 1.
	std::vector<double> account_balance(ballots.size(), 1);

	double minimum = 0, maximum = 100; // FIX LATER

	council_t council;

	for (size_t counter = 0; counter < council_size; ++counter) {
		// Bad programmer, bad! (Side effects ahoy.)
		council.push_back(elect_and_update(elected, account_balance,
				ballots, minimum, maximum,
				num_candidates, cumul_setting));
	}

	return (council);
}
