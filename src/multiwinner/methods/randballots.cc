#include "randballots.h"

#include <iterator>

#include <iostream>

std::list<size_t> random_ballots::get_council(size_t council_size,
	size_t num_candidates, const election_t & ballots) const {

	// https://stackoverflow.com/a/38158887
	std::vector<std::reference_wrapper<const ballot_group> >
	shuffled_ballots(ballots.cbegin(), ballots.cend());

	std::random_shuffle(shuffled_ballots.begin(),
		shuffled_ballots.end(), random_gen);

	std::vector<bool> hopefuls(num_candidates, true);

	bool elected_a_candidate;
	size_t num_elected = 0;
	std::list<size_t> elected_candidates;

	do {
		elected_a_candidate = false;

		for (const ballot_group & ballot: shuffled_ballots) {
			if (num_elected >= (size_t)council_size) {
				continue;
			}

			if (ballot.get_weight() != 1) {
				throw std::invalid_argument("Random ballots: "
					"weighted ballots are not supported.");
			}
			std::vector<size_t> winners = ordering_tools::get_winners(
					ballot.contents, hopefuls);

			std::random_shuffle(winners.begin(),
				winners.end(), random_gen);

			bool elected_from_ballot = false;

			for (size_t i = 0; i < winners.size()
				&& !elected_from_ballot; ++i) {

				if (!hopefuls[winners[i]]) {
					continue;
				}

				hopefuls[winners[i]] = false;
				elected_candidates.push_back(winners[i]);
				elected_from_ballot = true;
				elected_a_candidate = true;
				++num_elected;
			}
		}
	} while (elected_a_candidate && num_elected < (size_t)council_size);

	if (num_elected == (size_t)council_size) {
		return elected_candidates;
	}

	// Everybody's first preference has been accepted, but we still
	// haven't filled up the council. Just fill randomly.

	for (size_t i = 0; i < (size_t)num_candidates
		&& num_elected < (size_t)council_size; ++i) {

		if (hopefuls[i]) {
			hopefuls[i] = false;
			elected_candidates.push_back(i);
			++num_elected;
		}
	}

	return elected_candidates;
}