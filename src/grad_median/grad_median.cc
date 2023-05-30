// Data structure for "gradual median". See the headers.

#include <algorithm>
#include <iterator>

#include <assert.h>
#include <math.h>

#include <iostream>
#include <vector>
#include <queue>
#include <list>

#include "grad_median.h"


// Private

// This function finds the generalized median - the point directly under the
// sweep line's starting position. If the line goes right between two segments
// (like in the case with a median of 2), the return value is equal to the mean
// of the value just to the left and that just to the right (if they exist).

// Note that the function also modifies two of the input variables, the lower
// and upper, pointing to the lower and upper generalized bimedian (same unless
// it has to take the mean). It returns the point (sum of weights) where lower
// ends; that value is used to determine how much of the weight is to the left
// and the right of the sweep line in the algorithm itself.

double grad_fracile::get_center(
	std::list<std::pair<double, double> >::const_iterator & lower,
	std::list<std::pair<double, double> >::const_iterator & upper,
	const std::list<std::pair<double, double> > & source,
	double numvoters, double fracile) const {

	double sum = 0;

	// Find the lower "bimedian". If the central point is exactly between
	// the lower and upper, then upper is one block further, otherwise upper
	// is the same as lower.

	for (lower = source.begin(); lower != source.end() &&
		sum + lower->second < fracile * numvoters; ++lower) {
		sum += lower->second;
	}

	assert(lower != source.end());  // Happens if empty or wrong numvoters.

	upper = lower;

	// If it's exactly between two blocks (as in a median of 2), the upper
	// must be incremented unless that would lead upper to become invalid.

	if (sum + lower->second == fracile * numvoters) {
		++upper;
		if (upper == source.end()) {
			upper = lower;
		}
	}

	return (sum);
}

void grad_fracile::add_score(size_t candidate, double left, double right) {
	// It's private, so we presumably don't need this. Pass -1 to this
	// function on your own risk.
	/*if (candidate >= sorted_lists.size()) return;*/

	// TODO: Fix problem where, if one candidate has two 1 wt. blocks and
	// the other has one 1 wt. block, that will make the first win over the
	// second since scores are only added for those that change.

	// For now we'll just keep the edge scores. Magnitude-type Condorcet
	// might have a problem with this, so fix later.

	assert(right >= left);

	scores[candidate] = left + right;
}

// This function does run length encoding of the sorted list, so that if the
// number of ratings is fixed, the structure will be polyspace summable.

void grad_fracile::compress_candidate(std::list<std::pair<double, double> >
	&
	cand_scores, bool sorted) {

	if (!sorted) {
		cand_scores.sort();
	}

	std::list<std::pair<double, double> > ::iterator pos, advance;

	advance = cand_scores.begin();
	pos = advance++;

	while (advance != cand_scores.end()) {

		// If they're equal, remove the advance and fold weight into
		// the first.
		if (advance->first == pos->first) {
			pos->second += advance->second;
			advance = cand_scores.erase(advance);
		} else {
			++pos;
			++advance;
		}
	}

	does_need_reinit = true;
}

// -- //

grad_fracile::grad_fracile() {
	// The list, since it's empty, is "sorted". Nothing has been inited,
	// so we do need a reinit. The completion type defaults to GF_BOTH.

	does_need_reinit = true;
	all_sorted = true;

	completion = GF_BOTH;

	// Minimum and maximum scores are -infinity and +infinity respectively,
	// so that they are always below or above the most extreme rank
	// possible.

	minscorev.push_back(std::pair<double, double>(-INFINITY, INFINITY));
	maxscorev.push_back(std::pair<double, double>(INFINITY, INFINITY));

	num_hopefuls = 0;
}

int grad_fracile::add_candidate() {
	sorted_lists.push_back(std::list<std::pair<double, double> >());
	hopefuls.push_back(true);
	++num_hopefuls;

	finite_min_sentinels.push_back(std::list<std::pair<double, double> >());
	finite_min_sentinels.rbegin()->push_back(std::pair<double, double>(0,
			INFINITY));
	finite_max_sentinels.push_back(std::list<std::pair<double, double> >());
	finite_max_sentinels.rbegin()->push_back(std::pair<double, double>(0,
			INFINITY));
	left_indices.push_back(minscorev.begin());
	right_indices.push_back(minscorev.begin());

	scores.push_back(0);
	return (sorted_lists.size());
}

// Perhaps the other way is faster.
int grad_fracile::add_candidates(int how_many) {
	for (int counter = 0; counter < how_many; ++counter) {
		add_candidate();
	}

	return (get_num_candidates());
}

int grad_fracile::remove_last_candidate() {
	if (!sorted_lists.empty()) {
		if (*(hopefuls.rbegin())) {
			--num_hopefuls;
		}

		sorted_lists.pop_back();
		hopefuls.pop_back();
		scores.pop_back();
		finite_min_sentinels.pop_back();
		finite_max_sentinels.pop_back();
		left_indices.pop_back();
		right_indices.pop_back();
	}

	return (sorted_lists.size());
}

bool grad_fracile::add_rating(size_t candidate, double weight,
	double rating) {

	// If the candidate doesn't exist, fugeddaboudit.
	if (candidate >= sorted_lists.size()) {
		return (false);
	}

	// We can't permit inf/nan weights because they'll mess with the
	// sentinels.
	assert(finite(weight));

	// If it has a non-positive weight, do the same.
	if (weight <= 0) {
		return (false);
	}

	// If we go here, that means we're going to somehow incorporate the
	// new rating into the system, which means it'll need a reinit.
	// BLUESKY: Only set it if the sweep-line is past the point where it's
	// inserted, or a resort is needed.
	does_need_reinit = true;

	// If the candidate's entry is empty, insert and we're done.
	// Sorted remains true but a reset will be needed.
	if (sorted_lists[candidate].empty()) {
		sorted_lists[candidate].push_back(std::pair<double, double>(rating,
				weight));
		return (true);
	}

	// If the first block for this candidate has the same rating as what
	// we're trying to add, simply increment the weight.
	if (sorted_lists[candidate].begin()->first == rating) {
		sorted_lists[candidate].begin()->second += weight;
		return (true);
	}

	// Ditto for the last block.
	if (sorted_lists[candidate].rbegin()->first == rating) {
		sorted_lists[candidate].rbegin()->second += weight;
		return (true);
	}

	// If the rating is less than the first block, we can insert at the
	// beginning without turning the list unsorted, so do so.
	if (sorted_lists[candidate].begin()->first > rating) {
		sorted_lists[candidate].push_front(std::pair<double, double>(rating,
				weight));
		return (true);
	}

	// Same for last.
	if (sorted_lists[candidate].rbegin()->first < rating) {
		sorted_lists[candidate].push_back(std::pair<double, double>(rating,
				weight));
		return (true);
	}

	// Okay, so neither is true. Invalidate the sorted-order invariant
	// and insert at the end.
	all_sorted = false;
	sorted_lists[candidate].push_back(std::pair<double, double>(rating,
			weight));
	return (true);
}

// Same as add_rating, only clear the list first.
bool grad_fracile::set_rating(size_t candidate, double weight,
	double rating) {
	if (candidate >= sorted_lists.size()) {
		return (false);
	}

	sorted_lists[candidate].clear();
	return (add_rating(candidate, weight, rating));
}

bool grad_fracile::exclude(size_t candidate) {
	if (candidate >= sorted_lists.size()) {
		return (false);
	}

	if (hopefuls[candidate]) {
		--num_hopefuls;
	}

	hopefuls[candidate] = false;
	return (true);
}

bool grad_fracile::include(size_t candidate) {
	if (candidate >= sorted_lists.size()) {
		return (false);
	}

	if (hopefuls[candidate]) {
		return (true);
	}

	// Again, we could determine whether this would actually change
	// anything, before setting does_need_reinit. Bluesky: that, not worth
	// it for now.

	++num_hopefuls;
	hopefuls[candidate] = true;
	does_need_reinit = true;
	return (true);
}

bool grad_fracile::is_hopeful(size_t candidate) const {
	if (candidate >= sorted_lists.size()) {
		return (false);
	}

	return (hopefuls[candidate]);
}

int grad_fracile::get_num_hopefuls() const {
	return (num_hopefuls);
}

void grad_fracile::reset_hopefuls() {

	for (size_t counter = 0; counter < sorted_lists.size(); ++counter) {
		does_need_reinit |= !hopefuls[counter];
		hopefuls[counter] = true;
	}
}

double grad_fracile::get_score(size_t candidate) const {
	if (candidate >= sorted_lists.size()) {
		return (NAN);
	}

	return (scores[candidate]);
}

double grad_fracile::lget_eff_voters(size_t candidate) const {
	if (candidate >= sorted_lists.size()) {
		return (NAN);
	}

	double sum = 0;

	for (std::list<std::pair<double, double> >::const_iterator pos =
			sorted_lists[
				candidate].begin(); pos != sorted_lists[candidate].
		end(); ++pos) {
		sum += pos->second;
	}

	return (sum);
}


bool grad_fracile::init(double fracile, completion_type completion_in) {

	if (fracile < 0 || fracile > 1) {
		return (false);
	}

	// First set the completion type.
	completion = completion_in;

	size_t counter;

	// If we're not sorted yet, sort.
	if (!all_sorted) {
		for (counter = 0; counter < sorted_lists.size(); ++counter) {
			sorted_lists[counter].sort();
		}
		all_sorted = true;
	}

	// Clear the priority queue and iterators.
	left_pq = std::priority_queue<priority_entry, std::vector<priority_entry>,
	std::greater<priority_entry> >();
	right_pq = std::priority_queue<priority_entry, std::vector<priority_entry>,
	std::greater<priority_entry> >();

	left_indices.clear();
	right_indices.clear();

	// Reset scores and "changed" (since all scores are now 0).
	fill(scores.begin(), scores.end(), 0);
	changed = false;

	// For all candidates, find the generalized median (center point).
	std::list<std::pair<double, double> >::const_iterator cur_right, cur_left;
	double cumul, left_dist, right_dist;

	for (counter = 0; counter < sorted_lists.size(); ++counter) {
		// Get the number of voters.
		double numvoters = lget_eff_voters(counter);

		// If the candidate has no ratings at all, consider him to
		// rank below all others.
		if (sorted_lists[counter].empty()) {
			cur_left = minscorev.begin();
			cur_right = minscorev.begin();
			cumul = 0;
		} else
			// Otherwise, find the central point properly.
			cumul = get_center(cur_left, cur_right,
					sorted_lists[counter], numvoters,
					fracile);

		left_indices[counter] = cur_left;
		right_indices[counter] = cur_right;

		// If cur_left is not cur_right, the median is right between the
		// two and so the distance is 0.5 times the mean of those.
		// This is true even if fracile is not 0.5, because cur_left !=
		// cur_right means the central point is exactly between two
		// blocks.
		if (cur_left != cur_right) {
			left_dist = cur_left->second;
			right_dist = cur_right->second;
		} else {
			// If cur_left *is* cur_right, then the central point is
			// somewhere between. Find out how far it is by
			// determining the distance from numvoters * fracile to
			// each point.

			left_dist = numvoters * fracile - cumul;
			right_dist = cur_left->second - left_dist;

			// If it's empty, then these numbers make no sense:
			// substitute INFINITY instead.
			if (sorted_lists[counter].empty()) {
				left_dist = INFINITY;
				right_dist = INFINITY;
			}
		}

		add_score(counter, cur_left->first, cur_right->first);

		// Changed starts off as false because all are assumed to be
		// equal. Set it to true if not all are equal.

		if (counter > 0 && !changed) {
			changed = (get_score(counter) != get_score(counter-1));
		}

		// Insert the proper priority queue data for the sweep line
		// algorithm.
		left_pq.push(priority_entry(left_dist, counter,
				cur_left->first));
		right_pq.push(priority_entry(right_dist, counter,
				cur_right->first));

	}

	does_need_reinit = false;
	return (true);
}

bool grad_fracile::ordinal_changed() const {
	return (changed);
}

void grad_fracile::print_next_pair(size_t cand, bool left) const {
	if (left)
		std::cout << "(s: " << left_indices[cand]->first << ", len: " <<
			left_indices[cand]->second << ")";
	else
		std::cout << "(s: " << right_indices[cand]->first << ", len: " <<
			right_indices[cand]->second << ")";
}

// Move the sweep line and update relevant scores. This will return true if
// we managed to update something, otherwise false.
bool grad_fracile::update(bool debug) {

	// If there's nothing to work with, get outta here.
	if (sorted_lists.empty()) {
		return (false);
	}

	// If it needs to be inited, do so.
	if (does_need_reinit || !all_sorted)
		if (!init(0.5, completion)) {	// ???
			return (false);
		}

	// Okay, ready for update.
	// First remove not-hopefuls in either direction if there are any
	// hopefuls left.
	if (num_hopefuls < 2) {
		return (false);
	}

	while (!left_pq.empty() && !hopefuls[left_pq.top().array]) {
		left_pq.pop();
	}
	while (!right_pq.empty() && !hopefuls[right_pq.top().array]) {
		right_pq.pop();
	}

	// If either priority queue is empty, something is wrong; outta here.
	if (right_pq.empty() || left_pq.empty()) {
		if (debug) {
			std::cout << "ERROR: Empty priority queue!!" << std::endl;
		}
		return (false);
	}

	// Peek at the top of the queue to find the position/s with the shortest
	// distance.

	double minimum_distance = std::min(left_pq.top().distance,
			right_pq.top().distance);

	// If neither is finite, that means that we have a sentinel value; in
	// other words, that we've run out of data. In that case, there's
	// nothing left to do, so return false.
	if (!finite(minimum_distance)) {
		return (false);
	}

	if (debug) {
		std::cout << "Left minimum distance is " <<
			left_pq.top().distance << std::endl;
		std::cout << "Right minimum distance is " <<
			right_pq.top().distance << std::endl;
		std::cout << "Minimum distance is " << minimum_distance <<
			" with difference " << left_pq.top().distance -
			right_pq.top().distance << std::endl;
	}

	// Assume that no change in score has occurred. If that is still the
	// case after updating the distances, then we know no ordinal change
	// has happened. This is mostly useful in v_median where rank difference
	// is all that matters.
	changed = false;

	// Replace and update all minimum distance events on the left.
	priority_entry new_event;

	while (left_pq.top().distance == minimum_distance) {
		assert(!left_pq.empty());
		// Get and remove the old event.
		new_event = left_pq.top();
		left_pq.pop();

		int cand = new_event.array;

		// Update the array's iterator by moving it one step closer to
		// the start. If it's already at the start, add an infinity-
		// distance box instead. Which box depends on the completion.
		// If it's GF_BOTH or GF_LEAST, the score is -infinity,
		// otherwise it's the current rating (??)
		if (left_indices[cand] == sorted_lists[cand].begin()) {
			if (debug) {
				std::cout << cand << " reached beginning" << std::endl;
			}
			if (completion == GF_BOTH || completion == GF_LEAST) {
				left_indices[cand] = minscorev.begin();
			} else {
				if (debug)
					std::cout << "Setting min sentinel to "
						<< new_event.score << std::endl;
				// HACK HACK
				finite_min_sentinels[cand].begin()->first =
					new_event.score;
				left_indices[cand] =
					finite_min_sentinels[cand].begin();
			}
		} else {
			--left_indices[cand];
		}

		// Update distance and score, and reinsert.
		new_event.distance += left_indices[cand]->second;
		changed |= left_indices[cand]->first != new_event.score;
		new_event.score = left_indices[cand]->first;

		add_score(cand, new_event.score, right_indices[cand]->first);

		left_pq.push(new_event);

		if (debug)
			std::cout << "Updated candidate " << cand <<
				"'s left distance to " << new_event.distance <<
				std::endl;
	}


	// Same on the right.
	while (right_pq.top().distance == minimum_distance) {
		assert(!right_pq.empty());
		new_event = right_pq.top();
		right_pq.pop();

		int cand = new_event.array;

		++right_indices[cand];

		// If we're at the end and the completion is GF_BOTH or
		// GF_GREATEST, the value of the completion is infinity,
		// otherwise it's the current value.
		// Should this be -inf instead? Consider two cases:
		// 100 voters vote X: 10, and 101 voters vote Y: 10, shouldn't
		// Y win? Yes, but do that outside this class (tiebreak by
		// num voters). To do otherwise would mess with the result
		// if fracile != 0.5.
		// But consider also the Range worst case scenario: one person
		// gives ten points to unknown evildoer X. Then that will be
		// expanded to be like everybody gave X 10 points... (Only if
		// half vote every candidate zero.)
		if (right_indices[cand] == sorted_lists[cand].end()) {
			if (debug) {
				std::cout << cand << " reached end." << std::endl;
			}

			if (completion == GF_BOTH || completion == GF_GREATEST) {
				right_indices[cand] = maxscorev.begin();
			} else {
				if (debug)
					std::cout << "Setting max sentinel to "
						<< new_event.score << std::endl;
				// HACK HACK
				finite_max_sentinels[cand].begin()->first =
					new_event.score;
				right_indices[cand] = finite_max_sentinels[cand]
					.begin();
			}
		}

		new_event.distance += right_indices[cand]->second;
		changed |= right_indices[cand]->first != new_event.score;
		new_event.score = right_indices[cand]->first;

		add_score(cand, left_indices[cand]->first, new_event.score);

		right_pq.push(new_event);

		if (debug)
			std::cout << "Updated candidate " << cand <<
				"'s right distance to " << new_event.distance <<
				std::endl;
	}

	return (true);
}

void grad_fracile::compress() {

	for (size_t counter = 0; counter < sorted_lists.size(); ++counter) {
		compress_candidate(sorted_lists[counter], all_sorted);
	}

	all_sorted = true; // side effect.
}
