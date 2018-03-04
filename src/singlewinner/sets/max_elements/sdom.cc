
#include "sdom.h"

// Perhaps turn into matrix?

int sdom_set::strongly_dominates(int y, int x, const abstract_condmat & input, 
		const vector<bool> & hopeful) const {

	// Determine if dominator beats dominated. If so, continue; if it's
	// the other way around, then get the negative of the result with the
	// candidates swapped, and if they tie, then that's 0.

	double beat = input.get_magnitude(y, x, hopeful) - input.
		get_magnitude(x, y, hopeful);

	/*if (beat == 0)
		return(0);
	if (beat < 0)
		return(-strongly_dominates(x, y, input, hopeful));*/

	if (beat <= 0)
		return(0);

	// For each other candidate Z,
	bool still_dominates = true;

	for (int z = 0; z < input.get_num_candidates() && still_dominates; 
			++z) {
		// *other*
		if (z == x || z == y) continue;

		// if Z beats Y, Z beats X even more.
		if (input.get_magnitude(z, y, hopeful) > input.
				get_magnitude(y, z, hopeful))
			still_dominates &= (input.get_magnitude(z, x, hopeful) >
					input.get_magnitude(z, y, hopeful));

		if (!still_dominates) continue;

		// if Z beats X, Y beats X even more.
		if (input.get_magnitude(z, x, hopeful) > input.
				get_magnitude(x, z, hopeful))
			still_dominates &= (input.get_magnitude(y, x, hopeful) >
					input.get_magnitude(z, x, hopeful));

		if (!still_dominates) continue;

		// if X beats Z, Y beats Z even more.
		if (input.get_magnitude(x, z, hopeful) > 
				input.get_magnitude(z, x, hopeful))
			still_dominates &= (input.get_magnitude(y, z, hopeful) >
					input.get_magnitude(x, z, hopeful));

		if (!still_dominates) continue;
		// if Y beats Z, Y beats X even more.
		if (input.get_magnitude(y, z, hopeful) >
				input.get_magnitude(z, y, hopeful))
			still_dominates &= (input.get_magnitude(y, x, hopeful) >
					input.get_magnitude(y, z, hopeful));
	}

	if (still_dominates)
		return(1);
	else	return(0);
}

pair<ordering, bool> sdom_set::pair_elect(const abstract_condmat & input,
		const vector<bool> & hopefuls, cache_map * cache, 
		bool winner_only) const {

	// Make the empty "strongly dominates" matrix.
	condmat sdom_matrix(input.get_num_candidates(), input.get_num_voters(),
			CM_PAIRWISE_OPP);
	sdom_matrix.zeroize();

	vector<int> dominated(input.get_num_candidates(), 0);

	// Fill it with the results.
	int counter, sec;
	//cout << "Row dominates col" << endl;
	for (counter = 0; counter < input.get_num_candidates(); ++counter) {
		for (sec = 0; sec < input.get_num_candidates(); ++sec) {
			int result = 0;

			if (counter != sec)
				result = strongly_dominates(counter, sec, 
						input, hopefuls);
			if (result < 0) {
				++dominated[counter];
			sdom_matrix.add(counter, sec, result);
			assert (sdom_matrix.get_magnitude(counter, sec) == result);
			}
	//		cout << sdom_matrix.get_magnitude(counter, sec) << "\t";
			//sdom_matrix.add(sec, counter, -result);
		}
	//	cout << endl;
	}

	ordering toRet;

	for (counter = 0; counter < input.get_num_candidates(); ++counter)
		toRet.insert(candscore(counter, -dominated[counter]));

	return(pair<ordering, bool>(toRet, false));

	// Return the Smith set for this matrix.
	//return(pair<ordering,bool>(nested_sets(sdom_matrix, hopefuls), false));
}
