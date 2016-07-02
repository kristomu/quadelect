#include "partition.h"

using namespace std;

pair<ordering, bool> partition_set::pair_elect(const abstract_condmat & input,
		const vector<bool> & hopefuls, cache_map & cache,
		bool winner_only) const {

	// Start with a candidate schedule that goes 0..n. This means that
	// unless we randomize, 0 will be compared to 1, 2 to 3, etc.

	vector<int> schedule;
	schedule.reserve(input.get_num_candidates());

	int counter;
	// I *really* have to do something about hopefuls, it's way too risky.
	for (counter = 0; counter < input.get_num_candidates(); ++counter)
		if (hopefuls[counter])
			schedule.push_back(counter);

	// Randomize if required.
	if (is_random)
		random_shuffle(schedule.begin(), schedule.end());

	pair<ordering, bool> toRet;
	// Since it's a partition, it gives the same result whether or not
	// winner_only is true.
	toRet.second = false;

	// If it's odd, give the first candidate a bye.
	if (schedule.size() % 2 != 0) {
		toRet.first.insert(candscore(schedule[0], 1));
		counter = 1;
	} else
		counter = 0;

	while (counter < schedule.size()) {
		int prosp_winner = schedule[counter], 
		    prosp_loser = schedule[counter+1];

		if (input.get_magnitude(prosp_winner, prosp_loser, hopefuls) <
				input.get_magnitude(prosp_loser, prosp_winner,
					hopefuls))
			swap(prosp_winner, prosp_loser);

		toRet.first.insert(candscore(prosp_winner, 1));
		toRet.first.insert(candscore(prosp_loser, 0));

		counter += 2;
	}

	return(toRet);
}

void partition_set::set_random(bool be_random) {
	is_random = be_random;
	update_name();
}

string partition_set::pw_name() const {

	if (is_random)
		return("Partition/rand");
	else	return("Partition/set");
}

