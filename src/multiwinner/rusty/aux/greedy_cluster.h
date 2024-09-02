#pragma once

#include <algorithm>
#include <iostream>
#include <assert.h>
#include <vector>
#include <math.h>

using namespace std;

// Determine the best "equal size" clustering. This is a hack. It's also a
// greedy algorithm, and if this thing is NPC, that means we won't be optimal;
// but let's see how bad it is!

class es_sorter {

	public:
		vector<bool> full;

		int get_minimum(const vector<int>::const_iterator & begin,
			const vector<int>::const_iterator & end) const;

		bool operator()(const vector<vector<int> >::const_iterator & a,
			const vector<vector<int> >::const_iterator & b)
		const;
};


int es_sorter::get_minimum(const vector<int>::const_iterator & begin,
	const vector<int>::const_iterator & end) const {
	int record = *begin;
	int count = 0;

	for (vector<int>::const_iterator pos = begin; pos != end; ++pos) {
		if (*pos < record && !full[count]) {
			record = *pos;
		}
		++count;
	}

	return (record);
}

bool es_sorter::operator()(const vector<vector<int> >::const_iterator & a,
	const vector<vector<int> >::const_iterator & b) const {

	return (get_minimum(a->begin(), a->end()) <
			get_minimum(b->begin(), b->end()));
}

int get_clustering_error(const vector<vector<int> > & penalties,
	int num_clusters) {

	//cout << "---" << endl;

	// The discrepancy between lower and upper can be arbitrarily
	// assigned. (Oder? Spread as evenly as possible, perhaps?)

	int lower = floor(penalties.size()/(double)num_clusters);
	//int upper = ceil(penalties.size()/(double)num_clusters);
	int delta = penalties.size() - lower *
		num_clusters; // these can be assigned anywhere.

	vector<int> assigned(num_clusters, 0);
	vector<int> maxima(num_clusters, lower);
	vector<bool> cands_assigned(penalties.size(), false);
	es_sorter sorting;
	sorting.full = vector<bool>(num_clusters, false);
	int error = 0;

	// Sort by reference, by minimal error.
	// Maybe do faster with ints. Later.
	vector<vector<vector<int> >::const_iterator> queue(penalties.size());

	size_t counter = 0;

	for (counter = 0; counter < penalties.size(); ++counter) {
		queue[counter] = penalties.begin() + counter;
	}

	sort(queue.begin(), queue.end(), sorting);

	for (counter = penalties.size() - 1; counter >= 0; --counter) {
		// If any are now above the threshold, mark them as full
		// and redo sort based on this.

		int sec = 0;
		bool altered = false;
		for (sec = 0; sec < num_clusters; ++sec) {
			if (sorting.full[sec]) {
				continue;
			}
			//	cout << "Maxima for " << sec << " is " << maxima[sec] << endl;
			if (assigned[sec] >= maxima[sec] && delta <= 0) {
				//		cout << "!Set " << sec << " to done with delta " << delta << endl;
				sorting.full[sec] = true;
				altered = true;
			}
		}

		// Resort based only on those which are still free
		if (altered) {
			sort(queue.begin(), queue.begin() + counter, sorting);
		}

		// Determine which cluster has the minimum.
		// O(k), so sloow. Fix later!
		int record = 0;
		int recordholder = -1;
		sec = 0;
		for (vector<int>::const_iterator pos = queue[counter]->begin();
			pos != queue[counter]->end(); ++pos) {
			//cout << "Trying " << sec << endl;
			// Warning: ties
			if ((*pos < record || recordholder == -1) && !sorting.full[sec]) {
				record = *pos;
				recordholder = sec;
			}
			++sec;
		}

		assert(recordholder != -1);

		// Assign. (No need to do the actual assignment)
		error += record;
		assigned[recordholder]++;

		// If we would otherwise be full, withdraw from the delta
		// account.
		if (assigned[recordholder] > maxima[recordholder]) {
			maxima[recordholder]++;
			--delta;
			assert(delta >= 0);
		}
	}

	return (error);
}