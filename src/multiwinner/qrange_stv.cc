#include "qrange_stv.h"

std::vector<double> QRangeSTV::count_score(size_t num_candidates,
	const election_t & ballots,
	const std::list<double> & weights) const {

	std::list<double>::const_iterator wpos = weights.begin();
	election_t::const_iterator bgpos = ballots.begin();

	std::vector<double> score(num_candidates, 0);

	while (wpos != weights.end() && bgpos != ballots.end()) {
		for (ordering::const_iterator pos = bgpos->contents.begin();
			pos != bgpos->contents.end(); ++pos) {
			score[pos->get_candidate_num()] += *wpos *
				bgpos->get_weight() * pos->get_score();
		}

		++wpos;
		++bgpos;
	}

	assert(wpos == weights.end() && bgpos == ballots.end());

	return (score);
}

ordering QRangeSTV::get_possible_winners(size_t num_candidates,
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

// Todo: explicit spec of the upper and lower boundaries, e.g. for Range-20.

election_t QRangeSTV::normalize_ballots(const
	election_t & input) const {

	// Complexity? Don't ask me about complexity!

	election_t toRet;
	election_t::const_iterator pos;

	// First get the minimum and maximum values.

	assert(!input.empty());

	double minval = input.begin()->contents.begin()->get_score(),
		   maxval = input.begin()->contents.rbegin()->get_score();

	for (pos = input.begin(); pos != input.end(); ++pos) {
		maxval = std::max(pos->contents.begin()->get_score(), maxval);
		minval = std::min(pos->contents.rbegin()->get_score(), minval);
	}

	// Now actually normalize. This is probably n log n.

	for (pos = input.begin(); pos != input.end(); ++pos) {
		ballot_group dest = *pos;
		dest.contents.clear();
		for (ordering::const_iterator opos = pos->contents.begin();
			opos != pos->contents.end(); ++opos)
			dest.contents.insert(candscore(opos->
					get_candidate_num(),
					norm(minval, opos->
						get_score(),
						maxval)));
		toRet.push_back(dest);
	}

	return (toRet);
}

int QRangeSTV::elect_next(size_t council_size, size_t num_candidates,
	std::vector<bool> & elected,
	election_t & altered_ballots) const {

	// Get normal and quadratic count. The normal count is used for
	// determining if any candidate has a Droop quota. The quadratic
	// count is used for reweighting purposes.

	std::vector<double> count_ord(num_candidates, 0),
		count_quad(num_candidates, 0);

	election_t::const_iterator cpos;
	ordering::const_iterator ocpos;

	for (cpos = altered_ballots.begin(); cpos != altered_ballots.end();
		++cpos) {
		for (ocpos = cpos->contents.begin(); ocpos !=
			cpos->contents.end(); ++ocpos) {
			double adj_score = ocpos->get_score() * cpos->get_weight();

			count_ord[ocpos->get_candidate_num()] +=
				adj_score;
			count_quad[ocpos->get_candidate_num()] +=
				adj_score*ocpos->get_score();
		}
	}


	// Find winner.
	double record = -INFINITY;
	size_t winner = council_size, counter = 0; // the former is a sentinel

	// tiebreak? What's that? :p
	for (counter = 0; counter < count_ord.size(); ++counter)
		if (count_ord[counter] > record && !elected[counter]) {
			record = count_ord[counter];
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

	// For reweighting, we need the "magic variable" p_y s.th.
	//  sum a = 1..numvoters (vc_a_y^2 * p_y)) = (Droop quota).
	// which is equal to saying count_quad[y] * p_y = (Droop quota)
	// i.e...

	double p_y = quota / count_quad[winner];

	election_t::iterator pos = altered_ballots.begin();

	while (pos !=
		altered_ballots.end()) {
		double winner_score = 0;

		for (ordering::iterator opos = pos->contents.begin(); opos !=
			pos->contents.end() && winner_score == 0;
			++opos)
			if (opos->get_candidate_num() == winner) {
				winner_score = opos->get_score();
			}

		double subtr = pos->get_weight() * winner_score * p_y;

		if (pos->get_weight() - subtr < 0) {
			pos = altered_ballots.erase(pos);
		} else {
			pos->set_weight(std::max(0.0, pos->get_weight() - subtr));
			++pos;
		}
	}

	return (winner);
}

std::list<size_t> QRangeSTV::get_council(size_t council_size,
	size_t num_candidates,
	const election_t & ballots) const {

	election_t x = normalize_ballots(ballots);

	std::list<size_t> council;
	std::vector<bool> elected(num_candidates, false);

	for (size_t counter = 0; counter < council_size; ++counter) {
		// Ow, me side effects!
		council.push_back(elect_next(council_size - counter,
				num_candidates, elected, x));
	}

	return (council);
}
