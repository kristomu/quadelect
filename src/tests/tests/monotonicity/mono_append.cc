// Monotonicity: Mono-append. A method fails it if altering some truncated
// ballots that do not include X by adding X to the end, makes X lose.

#include "mono_append.h"

bool mono_append::alter_ballot(const ordering & input, ordering & output,
	int numcands, const vector<size_t> & data,
	rng & randomizer) const {

	// First check if our candidate is not on the ballot; if he isn't,
	// add him to the end.
	// "Lower" consists of removing the candidate if he's ranked last.

	int least_score = input.rbegin()->get_score();
	size_t cand_to_change = data[0];
	bool raise = (data[1] == 1);
	ordering::const_iterator cand_data = find_cand(input, cand_to_change);

	if (raise) {
		// If he isn't there, append!
		if (cand_data == input.end()) {
			output = input;
			output.insert(candscore(cand_to_change,
					least_score - 1));
			return (true);
		}

		// He's there, so we can't do anything.
		return (false);
	}

	// "Lower" - means we should remove the candidate if he ranks last.

	if (cand_data != input.end() && cand_data->get_score() == least_score) {
		output = input;
		bool found = false;
		for (ordering::iterator pos = output.begin();
			pos != output.end() && !found; ++pos)
			if (pos->get_candidate_num() == cand_to_change) {
				found = true;
				output.erase(pos);
				pos = output.begin();
			}

		return (found);
	}

	return (false);
}
