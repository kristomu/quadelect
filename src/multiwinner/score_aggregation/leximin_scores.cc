#include "leximin_scores.h"

ordinal leximin_scores::three_way_compare(
	const leximin_scores & other) const {

	std::map<double, double>::const_iterator our_pos, their_pos,
		our_next, their_next;

	our_pos = entries.begin();
	their_pos = other.entries.begin();

	our_next = entries.begin();
	their_next = other.entries.begin();

	// Here I'll be calling the score values "primary" and the
	// weights "secondary" to not confuse the score values (keys)
	// with map values (secondaries).

	// Since the score values are all unique, the algorithm is pretty
	// simple. We go down the sorted list of positions. If the (primary)
	// score values at our pos and their pos differ, then we're done.

	// Otherwise, if the weight differs, then whichever is greater must
	// be less than the other, if we're not on the last entry.
	// Since the score values are unique, it must be the case that if
	// our pair has a lower weight, that means there are fewer instances
	// of that score before it changes to something else, and that
	// something else must be a higher score -- unless it's the last
	// element, in which case we're just shorter.
	//
	// If both scores and values are the same, then we increment both
	// positions and repeat. If we reach the end without any resolution,
	// then they're equal. If we reach the end of *one* before the other,
	// then the one whose end we reach first is less than the other.

	while (our_pos != entries.end() && their_pos != other.entries.end()) {
		++our_next;
		++their_next;

		if (our_pos->first != their_pos->first) {
			if (our_pos->first < their_pos->first) {
				return LESS;
			}
			return GREATER;
		}

		if (our_pos->second != their_pos->second) {
			if (our_pos->second > their_pos->second) {
				// If we have a larger weight and they
				// don't have anything after this, their
				// list is shorter, so we're greater.
				if (their_next == other.entries.end()) {
					return GREATER;
				}
				// Otherwise, they transition to their next
				// value (which much be greater) quicker
				// than we do, so we're less.
				return LESS;
			} else {
				// Same logic in reverse.
				if (our_next == entries.end()) {
					return LESS;
				}
				return GREATER;
			}
		}

		our_pos = our_next;
		their_pos = their_next;
	}

	// They ran out of numbers before we did --> we're greater
	if (our_pos != entries.end()) {
		return GREATER;
	}

	// Vice versa their case.
	if (their_pos != other.entries.end()) {
		return LESS;
	}

	return EQUAL;
}

void leximin_scores::insert(double weight, double score) {
	std::map<double, double>::iterator score_entry = entries.find(score);

	if (score_entry == entries.end()) {
		entries[score] = weight;
	} else {
		score_entry->second += weight;
	}
}

bool leximin_scores::operator<(const leximin_scores & other) const {
	return three_way_compare(other) == LESS;
}

bool leximin_scores::operator<=(const leximin_scores & other) const {
	return three_way_compare(other) != GREATER;
}

bool leximin_scores::operator>(const leximin_scores & other) const {
	return three_way_compare(other) == GREATER;
}

bool leximin_scores::operator>=(const leximin_scores & other) const {
	return three_way_compare(other) != LESS;
}

bool leximin_scores::operator==(const leximin_scores & other) const {
	return three_way_compare(other) == EQUAL;
}

bool leximin_scores::operator!=(const leximin_scores & other) const {
	return three_way_compare(other) != EQUAL;
}
