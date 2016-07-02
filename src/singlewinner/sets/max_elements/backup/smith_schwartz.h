// <---> TOOD: SPLIT THIS
// Into det_sets, smith, schwartz, cdtt/cgtt, uncovered, sdom.

// These are classed as "election methods", but they produce ties so often
// that they really just elect sets.
// It's a point of question whether Copeland belongs in the "election method"
// list or here.
// WV, etc, doesn't matter here (AFAIK). For the classical ones, it doesn't,
// but it might for tourn-wv, etc. Therefore, always use WV for these (which
// we do)!

#ifndef _SET_SMITH
#define _SET_SMITH

#include "../method.cc"
#include "../pairwise/methods.cc"

class det_sets_relation {

	protected:
		virtual bool relation(const abstract_condmat & input, int a, 
				int b, const vector<bool> & hopefuls) const = 0;

		// Is this - to have nested sets - required? Make it an
		// option when invoking these set methods.

		ordering nested_sets(const abstract_condmat & input, 
				const vector<bool> & hopefuls, int limit) const;
		ordering nested_sets(const abstract_condmat & input,
				const vector<bool> & hopefuls) const {
			return(nested_sets(input, hopefuls,
						input.get_num_candidates()));}
};

// "limit" is the length limit of the path, used for calculating the Landau
// (Fishburn) set. A limit of (number of candidates) is equal to one of
// infinity.

ordering det_sets_relation::nested_sets(const abstract_condmat & input, 
		const vector<bool> & hopefuls, int limit) const {

	// N^3 (Floyd-Warshall)
	// If we only need the top set, we can do in N^2 (not implemented)
	
	int extent = input.get_num_candidates();

	// is extent if there's no path between x and y
	vector<vector<int> > path_len(extent, vector<int>(extent, extent));

	// Set up for paths of length 1
	int i, j, k;

	for (i = 0; i < extent; ++i)
		for (j = 0; j < extent; ++j)
			if (i != j && relation(input, i, j, hopefuls))
				path_len[i][j] = 1;

	// Handle intermediate paths
	for (k = 0; k < extent; ++k)
		for (i = 0; i < extent; ++i) {
			if (k == i) continue;
			for (j = 0; j < extent; ++j) {
				if (k == j || i == j) continue;
				if (path_len[i][j] > path_len[i][k] + 
						path_len[k][j])
					path_len[i][j] = path_len[i][k] +
						path_len[k][j];
			}
		}

	// Count.
	vector<int> path_score(extent, 0);

	for (i = 0; i < extent; ++i)
		for (j = 0; j < extent; ++j)
			if (i != j)
				if (path_len[i][j] > min(extent-1, limit))
					--path_score[i];

	// Finally, turn the count into an ordering.
	ordering toRet;

	for (i = 0; i < extent; ++i)
		toRet.insert(candscore(i, path_score[i]));

	return(toRet);
}

// See now how it becomes much simpler?

class smith_set : public pairwise_method, private det_sets_relation {

	private:
		bool relation(const abstract_condmat & input, int a, 
				int b, const vector<bool> & hopefuls) const;

	public:
		pair<ordering, bool> pair_elect(const abstract_condmat & input,
				const vector<bool> & hopefuls, 
				cache_map & cache, bool winner_only) const {
			return(pair<ordering,bool>(nested_sets(input,
							hopefuls), false));}

		smith_set() : pairwise_method(CM_WV) { update_name(); }

		string pw_name() const { return("Smith"); }

};

bool smith_set::relation(const abstract_condmat & input, int a, int b,
		const vector<bool> & hopefuls) const {
	// Was get_raw_contest_score. TODO, fix better.
	return(input.get_magnitude(a, b, hopefuls) >= 
			input.get_magnitude(b, a, hopefuls));
}

class schwartz_set : public pairwise_method, private det_sets_relation {

	private:
		bool relation(const abstract_condmat & input, int a,
				int b, const vector<bool> & hopefuls) const {
			return(input.get_magnitude(a, b, hopefuls) >
					input.get_magnitude(b, a, hopefuls));
		}

	public:
		pair<ordering, bool> pair_elect(const abstract_condmat & input,
				const vector<bool> & hopefuls,
				cache_map & cache, bool winner_only) const {
			return(pair<ordering,bool>(nested_sets(input,
							hopefuls), false));}

		schwartz_set() : pairwise_method(CM_WV) { update_name(); }

		string pw_name() const { return("Schwartz"); }
};

// I think? UNTESTED.

class cdtt_set : public pairwise_method, private det_sets_relation {
	private:
		bool relation(const abstract_condmat & input, int a, int b,
				const vector<bool> & hopefuls) const {
			return(input.get_magnitude(a, b, hopefuls) > input.
					get_num_voters() * 0.5);
		}
		
	public:
		pair<ordering, bool> pair_elect(const abstract_condmat & input,
				const vector<bool> & hopefuls,
				cache_map & cache, bool winner_only) const {
			return(pair<ordering,bool>(nested_sets(input,
							hopefuls), false));}

		cdtt_set() : pairwise_method(CM_WV) { update_name(); }

		string pw_name() const { return("CDTT"); }
};

class cgtt_set : public pairwise_method, private det_sets_relation {
        private:
                bool relation(const abstract_condmat & input, int a, int b,
                                const vector<bool> & hopefuls) const {
                        return(input.get_magnitude(a, b, hopefuls) >= input.
                                        get_num_voters() * 0.5);
                }

        public:
                pair<ordering, bool> pair_elect(const abstract_condmat & input,
                                const vector<bool> & hopefuls,
                                cache_map & cache, bool winner_only) const {
                        return(pair<ordering,bool>(nested_sets(input,
                                                        hopefuls), false));}

                cgtt_set() : pairwise_method(CM_WV) { update_name(); }

                string pw_name() const { return("CGTT"); }
};

class landau_set : public pairwise_method, private det_sets_relation {
	private:
		bool relation(const abstract_condmat & input, int a, int b,
				const vector<bool> & hopefuls) const {
			return(input.get_magnitude(a, b, hopefuls) >=
					input.get_magnitude(b, a, hopefuls));
		}

	public:
		pair<ordering, bool> pair_elect(const abstract_condmat & input,
				const vector<bool> & hopefuls,
				cache_map & cache, bool winner_only) const {
			return(pair<ordering,bool>(nested_sets(input,
							hopefuls, 2), false));}

		landau_set() : pairwise_method(CM_WV) { update_name(); }

		string pw_name() const { return("Landau"); }
};

// Strongly undominated set. This is simply the set of those who aren't 
// strongly dominated by another candidate. The iterated set is done in a
// Copeland-ish manner.

// Y strongly dominates X if
//  1. Y beats X
// and, for all Z distinct from X and Y,
//  2. if Z beats Y, Z beats X even more,
//  3. if Z beats X, Y beats X even more,
//  4. if X beats Z, Y beats Z even more,
//  5. if Y beats Z, Y beats X even more.

// TODO: Move this off or use some more sophisticated method to account for
// e.g. if A is dominated by 1 and B is dominated by A, then ... > A > B, not
// ... > A = B.

class sdom_set : public pairwise_method, private det_sets_relation {
	private:

		// 1 = dominates, -1 = dominated, 0 = nondominated
		int strongly_dominates(int dominator, int dominated,
				const abstract_condmat & input,
				const vector<bool> & hopefuls) const;

		// Relation is the same as for the Smith set, but on a 
		// different matrix.
		bool relation(const abstract_condmat & input, int a,
				int b, const vector<bool> & hopefuls) const {
			return(input.get_magnitude(a, b, hopefuls) >=
					input.get_magnitude(b, a, hopefuls));
		}

	public:
		pair<ordering, bool> pair_elect(const abstract_condmat & input,
				const vector<bool> & hopefuls,
				cache_map & cache, bool winner_only) const;

		sdom_set() : pairwise_method(CM_WV) { update_name(); }

		string pw_name() const { return("SDom"); }

};

int sdom_set::strongly_dominates(int y, int x, const abstract_condmat & input, 
		const vector<bool> & hopeful) const {

	// Determine if dominator beats dominated. If so, continue; if it's
	// the other way around, then get the negative of the result with the
	// candidates swapped, and if they tie, then that's 0.

	double beat = input.get_magnitude(y, x, hopeful) - input.
		get_magnitude(x, y, hopeful);

	if (beat == 0)
		return(0);
	if (beat < 0)
		return(-strongly_dominates(x, y, input, hopeful));

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
		const vector<bool> & hopefuls, cache_map & cache, 
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

#endif
