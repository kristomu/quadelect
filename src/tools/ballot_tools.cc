#include <iostream>
#include <list>
#include "../ballots.h"
#include "ballot_tools.h"

#include <iterator>

// TODO: Handle more than 26 candidates??
std::map<size_t, std::string> get_default_candidate_labeling() {
	std::map<size_t, std::string> fakecand;

	std::string f = "!";

	for (int counter = 0; counter < 26; ++counter) {
		f[0] = (char)('A' + counter);
		fakecand[counter] = f;
	}

	return fakecand;
}

// Ordering sorter
int ordering_sorter::compare(const ordering & a,
	const ordering & b) const {

	// EOF precedes any candidate number.
	// Otherwise, leximax on the candidates. If there's a tie, break on
	// scores, earliest difference first.

	//int asize = a.size(), bsize = b.size();
	// Uncomment for a "breadth-first" effect.
	/*if (asize < bsize) return(-1);
	if (bsize > asize) return(1);*/

	ordering::const_iterator apos = a.begin(), bpos = b.begin();
	int tiebreaker = 0; // equal

	while (apos != a.end() && bpos != b.end()) {
		if (tiebreaker == 0) {
			if (apos->get_score() < bpos->get_score()) {
				tiebreaker = -1;
			}
			if (apos->get_score() > bpos->get_score()) {
				tiebreaker = 1;
			}
		}

		if (apos->get_candidate_num() < bpos->get_candidate_num()) {
			return (-1);
		}
		if (apos->get_candidate_num() > bpos->get_candidate_num()) {
			return (1);
		}

		++apos;
		++bpos;
	}

	int asize = a.size(), bsize = b.size();
	if (asize < bsize) {
		return (-1);
	}
	if (bsize > asize) {
		return (1);
	}

	return (tiebreaker);
}

bool ordering_sorter::operator()(const ordering & a,
	const ordering & b) const {
	// Equality returns both a < b and b < a true.
	return (compare(a, b) < 1);
}

bool ordering_sorter::operator()(const ballot_group & a,
	const ballot_group & b) const {
	// If it's a tie, break further on size.
	// We can't sort by size first, because then merging identical ballots
	// wouldn't work, and that's something we'd prefer to work.

	int dir = compare(a.contents, b.contents);

	if (dir == 0) {
		return (a.get_weight() < b.get_weight());
	} else	{
		return (dir < 1);
	}
}

// The ordering tools themselves.

ordering ordering_tools::reverse(const ordering & in) const {

	ordering out;

	for (ordering::const_iterator pos = in.begin(); pos != in.end(); ++pos)
		out.insert(candscore(pos->get_candidate_num(),
				-pos->get_score()));

	return (out);
}

ordering ordering_tools::winner_only(const ordering & in) const {

	ordering out;

	for (ordering::const_iterator pos = in.begin(); pos != in.end() &&
		pos->get_score() == in.begin()->get_score(); ++pos) {
		out.insert(*pos);
	}

	return (out);
}

// Turns it into 0 for first place, -1 for second, etc.
ordering ordering_tools::scrub_scores(const ordering & in) const {

	int count = -1;
	double last = -INFINITY;

	ordering out;

	for (ordering::const_iterator pos = in.begin(); pos != in.end(); ++pos) {
		if (pos == in.begin() || pos->get_score() != last) {
			++count;
		}
		out.insert(candscore(pos->get_candidate_num(), -count));
		last = pos->get_score();
	}

	return (out);
}

ordering ordering_tools::scrub_scores_by_cand(const ordering & in) const {

	int count = -1;
	double last = -INFINITY;
	double intermed = -1;

	ordering out;

	for (ordering::const_iterator pos = in.begin(); pos != in.end(); ++pos) {
		++intermed;
		if (pos == in.begin() || pos->get_score() != last) {
			count = intermed;
		}
		out.insert(candscore(pos->get_candidate_num(), -count));
		last = pos->get_score();
	}

	return (out);
}

bool ordering_tools::has_multiple_winners(const ordering & in) {

	for (ordering::const_iterator pos = in.begin(); pos != in.end() &&
		pos->get_score() == in.begin()->get_score(); ++pos) {

		// If the candidate at the current position has the same
		// score as the winner but isn't the winner, then we have
		// a tie.
		if (pos != in.begin()) {
			return true;
		}
	}

	return false;
}

std::list<int> ordering_tools::get_winners(const ordering & in) {

	std::list<int> winners;

	for (ordering::const_iterator pos = in.begin(); pos != in.end() &&
		pos->get_score() == in.begin()->get_score(); ++pos) {
		winners.push_back(pos->get_candidate_num());
	}

	return (winners);
}

bool ordering_tools::is_winner(const ordering & in,
	int candidate_num) {

	// This may be slower than an optimized loop that aborts once
	// the winner has been found, but deal with that later if required.
	std::list<int> winners = get_winners(in);

	return std::find(winners.begin(), winners.end(), candidate_num)
		!= winners.end();
}

std::list<candscore> ordering_tools::get_loser_candscores(
	const ordering & in) {

	std::list<candscore> losers;

	for (ordering::const_reverse_iterator rpos = in.rbegin();
		rpos != in.rend() && rpos->get_score() == in.rbegin()->get_score();
		++rpos) {

		losers.push_back(*rpos);
	}

	return losers;
}

// Break ties in the ordering "tied" according to tiebreaker.
// This is done by finding the minimal delta (difference between two candidates)
// and rescaling the tiebreaker so that the difference between first and last is
// equal to the delta divided by number of specified candidates. Picking that
// interval means that there's no way we'll disturb the original order; adding
// deltas will only disambiguate equals. Then we proceed down the original
// ballot, adding deltas accordingly.
ordering ordering_tools::tiebreak(const ordering & tied,
	const ordering & tiebreaker, size_t num_candidates) const {

	// Some simple checks.
	// If tied is of size 1, then there's nothing to do - the tie is
	// already maximally broken.
	if (tied.size() == 1) {
		return (tied);
	}

	std::vector<double> tiebreaker_diff(num_candidates, 0);

	// Find minimum and maximum for tiebreaker
	// Well, those are just tiebreaker.rbegin() and tiebreaker.begin() resp.

	// Find smallest nonzero delta between candidates in tied.
	// Start with the greatest possible delta.
	double delta = tied.begin()->get_score() - tied.rbegin()->get_score();

	// If it's zero, there's a tie among all candidates, so just set
	// the delta to 1 instead. Note that we can't just return the tiebreak:
	// there may be more candidates in the tiebreak than in tied.
	if (delta == 0) {
		delta = 1;
	}

	// Otherwise, try to find the smallest.
	ordering::const_iterator pos = tied.begin(), last = tied.begin();

	while (pos != tied.end()) {
		if (pos == last) {
			++pos;
			continue;
		}

		if (last->get_score() - pos->get_score() < delta &&
			last->get_score() - pos->get_score() > 0) {
			delta = last->get_score() - pos->get_score();
		}

		++pos;
		++last;
	}

	delta /= (double)num_candidates;

	// Now dump the tiebreaker values to an array so we can do random access
	// on them. Note that candidates listed in tied but not tiebreaker will
	// have last rank (as we wanted), since those are inited to 0 and the
	// renorm ensures "real" scores will be above 0.01 deltas.

	for (pos = tiebreaker.begin(); pos != tiebreaker.end(); ++pos) {
		assert(pos->get_candidate_num() < num_candidates);

		tiebreaker_diff[pos->get_candidate_num()] = renorm(
				tiebreaker.rbegin()->get_score(),
				tiebreaker.begin()->get_score(),
				pos->get_score(),
				0.01 * delta, 1.0 * delta);
	}

	// Finally, construct our tie-adjusted ordering.
	// BLUESKY: Perhaps normalize tied to 0...1 instead for numerical
	// accuracy purposes?

	ordering toRet;

	for (pos = tied.begin(); pos != tied.end(); ++pos)
		toRet.insert(candscore(pos->get_candidate_num(),
				pos->get_score() +
				tiebreaker_diff[pos->
					get_candidate_num()]));

	// Done!

	return (toRet);
}

ordering ordering_tools::ranked_tiebreak(const ordering & tied,
	const ordering & tiebreaker, size_t num_candidates) {

	// This works by sorting a struct of ints, which includes a pair.
	// The first int in the pair is the order of the candidate according
	// to the first ordering, and the second into according to the second.

	// TODO: This is a horrible mess. Clean it up.

	size_t counter;
	bool debug = false;

	std::map<size_t, std::string> fakecand;
	if (debug) {
		// DEBUG.
		for (counter = 0; counter < 26; ++counter) {
			fakecand[counter] = (char)('A' + counter);
		}

		std::cout << "Tied: " << ordering_to_text(tied, true)
			<< std::endl;
		std::cout << "Tiebreak: " << ordering_to_text(
				tiebreaker, true) << std::endl;
	}

	typedef std::pair<std::pair<int, int>, int> sorter;
	// We don't want candidates who aren't used in the tied ballot to be
	// on the result ballot.
	std::vector<bool> used_candidate(num_candidates, false);

	std::vector<sorter> to_sort(num_candidates);

	for (counter = 0; counter < to_sort.size(); ++counter) {
		to_sort[counter].second = counter;
		to_sort[counter].first = std::pair<int, int>(-10, -10);
	}

	ordering::const_reverse_iterator rpos = tied.rbegin(), old_pos =
			rpos;

	// Insert the first, manually.
	int rank_count = 0;
	to_sort[rpos->get_candidate_num()].first.first = rank_count;
	used_candidate[rpos->get_candidate_num()] = true;
	++rpos;

	while (rpos != tied.rend()) {
		if (rpos->get_score() != old_pos->get_score()) {
			++rank_count;
		}
		to_sort[rpos->get_candidate_num()].first.first = rank_count;
		// This candidate is in use, so mark it that way.
		used_candidate[rpos->get_candidate_num()] = true;
		old_pos = rpos++;
	}

	// Then handle the second.
	rank_count = 0;
	rpos = tiebreaker.rbegin();
	old_pos = rpos;
	to_sort[rpos->get_candidate_num()].first.second = rank_count;
	++rpos;

	while (rpos != tiebreaker.rend()) {
		if (rpos->get_score() != old_pos->get_score()) {
			++rank_count;
		}
		to_sort[rpos->get_candidate_num()].first.second = rank_count;
		old_pos = rpos++;
	}

	// Sort...

	sort(to_sort.begin(), to_sort.end(), std::greater<sorter>());

	// And read off.

	ordering output;
	rank_count = 0;
	std::vector<sorter>::const_reverse_iterator spos = to_sort.rbegin(),
												old_spos = spos;
	if (used_candidate[spos->second]) {
		output.insert(candscore(spos->second, rank_count));
	}
	++spos;

	while (spos != to_sort.rend()) {
		if (spos->first != old_spos->first) {
			++rank_count;
		}
		if (used_candidate[spos->second]) {
			output.insert(candscore(spos->second, rank_count));
		}
		old_spos = spos++;
	}

	if (debug)
		std::cout << "Output: " << ordering_to_text(output, fakecand, true)
			<< std::endl;

	return (output);
}

bool ordering_tools::has_equal_rank(const ordering & to_check) const {

	// Can't this be replaced by
	//	return (to_check.begin()->get_score() ==
	//		to_check.rbegin()->get_score())  ?

	ordering::const_iterator pos = to_check.begin(), pos_old;
	bool equal_rank = false;
	pos_old = pos++;

	while (!equal_rank && pos != to_check.end()) {
		equal_rank = pos->get_score() == pos_old->get_score();
		pos_old = pos++;
	}

	return (equal_rank);
}

std::string ordering_tools::ordering_to_text(const ordering & rank_ballot,
	const std::map<size_t, std::string> & reverse_cand_lookup,
	bool numeric) {

	double oldscore = -INFINITY;
	bool first = true;

	std::string out;

	for (ordering::const_iterator pos = rank_ballot.begin(); pos !=
		rank_ballot.end(); ++pos) {
		if (reverse_cand_lookup.find(pos->get_candidate_num()) ==
			reverse_cand_lookup.end()) {
			return ("ERR");
		} else {
			double newscore = pos->get_score();

			if (!first) {
				if (newscore < oldscore) {
					out += " > ";
				} else {
					out += " = ";
				}
			} else {
				first = false;
			}

			out += reverse_cand_lookup.find(
					pos->get_candidate_num())->second;
			if (numeric) {
				out += "(" + dtos(pos->get_score()) + ")";
			}

			oldscore = newscore;
		}
	}

	return (out);
}

std::string ordering_tools::ordering_to_text(const ordering & rank_ballot,
	bool numeric) {

	return ordering_to_text(rank_ballot,
			get_default_candidate_labeling(), numeric);
}

ordering ordering_tools::direct_vector_to_ordering(const
	std::vector<double> & in,
	const std::vector<bool> & hopefuls) const {

	ordering toRet;

	for (size_t counter = 0; counter < std::min(in.size(), hopefuls.size());
		++counter)
		if (hopefuls[counter]) {
			toRet.insert(candscore(counter, in[counter]));
		}

	return (toRet);
}

ordering ordering_tools::indirect_vector_to_ordering(
	const std::vector<double> & in,
	const std::vector<int> & mapping) const {

	ordering toRet;

	for (size_t counter = 0; counter < mapping.size(); ++counter) {
		toRet.insert(candscore(mapping[counter], in[counter]));
	}

	return (toRet);
}

///// Ballot tools.
//

ballot_group ballot_tools::truncate_after(const ballot_group & in,
	size_t truncate_after_candidate_num) {

	ordering truncated_ballot;
	bool found_threshold = false;
	for (ordering::const_iterator pos = in.contents.begin();
		pos != in.contents.end() && !found_threshold;
		++pos) {
		truncated_ballot.insert(*pos);
		found_threshold |= (pos->get_candidate_num() ==
				truncate_after_candidate_num);
	}

	bool ballot_is_complete = in.complete && (
			in.contents.size() == truncated_ballot.size());

	return ballot_group(in.get_weight(), truncated_ballot,
			ballot_is_complete, in.rated);
}

election_t ballot_tools::truncate_after(
	const election_t & ballots,
	size_t truncate_after_candidate_num) {

	election_t truncated_ballots;

	for (const ballot_group & group: ballots) {
		truncated_ballots.push_back(truncate_after(group,
				truncate_after_candidate_num));
	}

	return truncated_ballots;
}

election_t ballot_tools::sort_ballots(const
	election_t & to_sort) const {
	election_t sorted = to_sort;

	sorted.sort(otools.sorter);

	return (sorted);
}


election_t ballot_tools::compress(const
	election_t &
	uncompressed) const {

	// You asked for n log n, here it is.

	if (uncompressed.empty()) {
		return (uncompressed);
	}

	election_t compressed = sort_ballots(uncompressed);
	election_t::iterator check, check_against;

	for (check = compressed.begin(); check != compressed.end(); ++check) {
		check_against = check;
		++check_against;
		// Remember: always put the check for end first so that &&
		// breaks on it, otherwise same_rank might try to access
		// something that doesn't exist, leading to big bara boom.
		while (check_against != compressed.end() &&
			check->contents == check_against->contents) {

			check->set_weight(check->get_weight() +
				check_against->get_weight());
			check_against = compressed.erase(check_against);
		}
	}

	return (compressed);
}

std::string ballot_tools::ballot_to_text(const ballot_group & rank_ballot,
	const std::map<size_t, std::string> & reverse_cand_lookup,
	bool numeric) const {

	std::string out = dtos(rank_ballot.get_weight()) + ": ";
	std::string order = otools.ordering_to_text(rank_ballot.contents,
			reverse_cand_lookup, numeric);

	if (order == "ERR") {
		return (order);
	} else	{
		return (out + order);
	}
}

std::vector<std::string> ballot_tools::ballots_to_text(std::string prefix,
	const election_t & rank_ballots,
	const std::map<size_t, std::string> & reverse_cand_lookup,
	bool numeric) const {

	std::vector<std::string> output;

	for (election_t::const_iterator pos = rank_ballots.begin();
		pos != rank_ballots.end(); ++pos) {
		output.push_back(prefix + ballot_to_text(*pos,
				reverse_cand_lookup, numeric));

		if (*output.rbegin() == "ERR") {
			return (std::vector<std::string>(1, "ERR"));
		}
	}

	return (output);
}

std::vector<std::string> ballot_tools::ballots_to_text(
	const election_t &
	rank_ballots, const std::map<size_t, std::string> & reverse_cand_lookup,
	bool numeric) const {

	return (ballots_to_text("", rank_ballots, reverse_cand_lookup, numeric));
}

void ballot_tools::print_ranked_ballots(const election_t &
	rank_ballots) const {

	std::map<size_t, std::string> fakecand = get_default_candidate_labeling();

	std::vector<std::string> fv = ballots_to_text(rank_ballots, fakecand,
			false);
	copy(fv.begin(), fv.end(), std::ostream_iterator<std::string>(std::cout,
			"\n"));
}

election_t ballot_tools::rescale(
	const election_t & ballots, double factor) const {
	election_t out;

	for (ballot_group ballot: ballots) {
		ballot.set_weight(ballot.get_weight() * factor);
		out.push_back(ballot);
	}

	return out;
}