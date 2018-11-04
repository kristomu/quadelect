// See the header for more info.

#include "eligibility.h"

void eligibility_table::mark_eligible(algo_t first, algo_t second,
	bool strict_inequality) {

	// If (first, second) doesn't already exist, we need to add it to
	// our structures.

	if (eligible[first].count(second) == 0) {
		eligible[first].insert(second);
		status[std::pair<algo_t, algo_t>(first, second)] = 
			eligibility_status();
		eligibles_kth_column[0][first]++;
		eligibles_kth_column[1][second]++;
	}

	status[std::pair<algo_t, algo_t>(first, second)].any_strict_inequality |=
		strict_inequality;
}

void eligibility_table::mark_ineligible(algo_t first, algo_t second) {

	if (eligible[first].count(second) == 0) {
		throw std::runtime_error(
			"Tried to mark ineligible something not eligible");
	}

	eligibles_kth_column[0][first]--;
	eligibles_kth_column[1][second]--;

	eligible[first].erase(second);

	if (eligibles_kth_column[1][second] == 0) {
		eligibles_kth_column[1].erase(second);
	}

	// if (first, *) has count 0, we can remove first from the eligible
	// list altogether as it has no descendants.
	if (eligibles_kth_column[0][first] == 0) {
		eligibles_kth_column[0].erase(first);
		eligible.erase(first);
	}
}