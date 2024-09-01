#include "range_stv.h"

std::vector<double> LRangeSTV::count_score(size_t num_candidates,
	const election_t & ballots,
	const std::list<double> & weights) const {

	std::list<double>::const_iterator wpos = weights.begin();
	election_t::const_iterator bgpos = ballots.begin();

	std::vector<double> score(num_candidates, 0);

	while (wpos != weights.end() && bgpos != ballots.end()) {
		for (ordering::const_iterator pos = bgpos->contents.begin();
			pos != bgpos->contents.end(); ++pos)
			score[pos->get_candidate_num()] += *wpos *
				bgpos->get_weight() * pos->get_score();

		++wpos;
		++bgpos;
	}

	assert(wpos == weights.end() && bgpos == ballots.end());


	return (score);
}

ordering LRangeSTV::get_possible_winners(size_t num_candidates,
	const election_t & ballots,
	const std::list<double> & weights) const {

	std::vector<double> raw_score = count_score(num_candidates, ballots,
			weights);

	ordering out;

	for (size_t counter = 0; counter < raw_score.size(); ++counter) {
		out.insert(candscore(counter, raw_score[counter]));
	}

	return (out);
}

int LRangeSTV::elect_next(size_t council_size, size_t num_candidates,
	std::vector<bool> & elected,
	election_t & altered_ballots) const {

	// Get count.
	std::list<double> weights(altered_ballots.size(), 1);

	std::vector<double> count = count_score(num_candidates,
			altered_ballots, weights);

	// Find winner.
	double record = -INFINITY;
	size_t winner = council_size, counter = 0; // the former is a sentinel

	// tiebreak? What's that? :p
	for (counter = 0; counter < count.size(); ++counter)
		if (count[counter] > record && !elected[counter]) {
			record = count[counter];
			winner = counter;
		}

	elected[winner] = true;

	// If that was the last one, then there's no need to reweight.
	if (council_size == 1) {
		return (winner);
	}

	// Get number of voters and reweight all.

	double num_voters = 0;

	for (election_t::const_iterator cpos = altered_ballots.begin();
		cpos != altered_ballots.end(); ++cpos) {
		num_voters += cpos->get_weight();
	}

	double quota = num_voters / (num_candidates+1.0);

	election_t::iterator pos = altered_ballots.begin();

	while (pos != altered_ballots.end()) {
		double winner_score = 0;

		for (ordering::iterator opos = pos->contents.begin(); opos !=
			pos->contents.end() && winner_score == 0;
			++opos)
			if (opos->get_candidate_num() == winner) {
				winner_score = opos->get_score();
			}

		winner_score *= pos->get_weight();

		// Adjust ballot
		double prop = winner_score / count[winner];

		double factor = (pos->get_weight() - quota*prop)/pos->get_weight();

		// Should a negative factor even be possible??
		if (factor <= 0) {
			pos = altered_ballots.erase(pos);
		} else {
			pos->set_weight(pos->get_weight() * std::max(0.0, factor));
			++pos;
		}
	}

	return (winner);
}

std::list<size_t> LRangeSTV::get_council(size_t council_size,
	size_t num_candidates,
	const election_t & ballots) const {

	election_t x = ballots;

	std::list<size_t> council;
	std::vector<bool> elected(num_candidates, false);

	for (size_t counter = 0; counter < council_size; ++counter) {
		// Ow, me side effects!
		council.push_back(elect_next(council_size - counter,
				num_candidates, elected, x));
	}

	return (council);
}
