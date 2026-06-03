#include "coalition.h"

// Hopefully quicker! DAC style for now, do DSC later.
// This is slow if there are lots of candidates and truncation going on, e.g.
// 65000 candidates and everybody bullet-votes. In that case, set will generate
// 1+2+..+65000 ints per, this will generate 65000^2 bits per.
// It would be better to post-generate the "done" ballots, but is that worth it?
// I.e. count AB|, C|, AC|, then later complete these in one swoop each... Hm.
// DSC doesn't have to deal with that.

struct simple_hash {

	size_t operator()(const vector<bool> & a) const {

		size_t ct = 2166136261;//1337;

		// bad hash. Just to check!
		int counter = 1;
		for (vector<bool>::const_iterator p = a.begin(); p != a.end();
			++p) {
			if (*p) {
				ct ^= counter;
			}
			ct *= 16777619;
			++counter;
		}

		return (ct);
	}
};

vector<solid_coalition> get_acquiescing_coalitions(
	const list<ballot_group> & input, int num_candidates) {

	// For each ballot group,
	// 	For each candidate,
	// 		Add that candidate to the set
	// 		Increment the map to the set by the group's power.
	// 	Next
	// 	Clear the set
	// Next

	// Might try sorting by candidate, later, to see if it helps.

	std::unordered_map<vector<bool>, double, simple_hash> forwards;
	vector<bool> current_coal(num_candidates, false);
	vector<bool> seen_candidates(num_candidates, false); // Used for DAC

	for (list<ballot_group>::const_iterator pos = input.begin(); pos !=
		input.end(); ++pos) {

		fill(current_coal.begin(), current_coal.end(), false);

		for (ordering::const_iterator inner_pos = pos->contents.begin();
			inner_pos != pos->contents.end(); ++inner_pos) {
			current_coal[inner_pos->get_candidate_num()] = true;
			forwards[current_coal] += pos->get_weight();
		}

		// Now add all remaining candidates, one at a time.
		for (vector<bool>::iterator cpos = current_coal.begin(); cpos
			!= current_coal.end(); ++cpos) {
			if (*cpos) {
				continue;
			}
			*cpos = true;
			forwards[current_coal] += pos->get_weight();
		}
	}

	// Dump to a vector.
	vector<solid_coalition> output;
	output.reserve(forwards.size());
	solid_coalition to_add;

	for (std::unordered_map<vector<bool>, double, simple_hash>::
		const_iterator mpos = forwards.begin();
		mpos != forwards.end(); ++mpos) {
		to_add.candidates.clear();
		int rictr = 0;
		for (vector<bool>::const_iterator cipos = mpos->first.begin();
			cipos != mpos->first.end(); ++cipos) {
			if (*cipos) {
				to_add.candidates.insert(rictr);
			}
			++rictr;
		}
		to_add.support = mpos->second;

		output.push_back(to_add);
	}

	// Finally, sort.
	sort(output.begin(), output.end(), greater<solid_coalition>());

	return (output);
}
