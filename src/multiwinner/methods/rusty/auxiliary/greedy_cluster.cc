#include "greedy_cluster.h"

int es_sorter::get_minimum(const std::vector<int>::const_iterator & begin,
	const std::vector<int>::const_iterator & end) const {
	int record = *begin;
	int count = 0;

	for (std::vector<int>::const_iterator pos = begin; pos != end; ++pos) {
		if (*pos < record && !full[count]) {
			record = *pos;
		}
		++count;
	}

	return (record);
}

bool es_sorter::operator()(const
	std::vector<std::vector<int> >::const_iterator & a,
	const std::vector<std::vector<int> >::const_iterator & b) const {

	return (get_minimum(a->begin(), a->end()) <
			get_minimum(b->begin(), b->end()));
}

int es_sorter::get_clustering_error(
	const std::vector<std::vector<int> > & penalties, int num_clusters) {

	//cout << "---" << endl;

	// The discrepancy between lower and upper can be arbitrarily
	// assigned. (Oder? Spread as evenly as possible, perhaps?)

	int lower = floor(penalties.size()/(double)num_clusters);
	//int upper = ceil(penalties.size()/(double)num_clusters);
	int delta = penalties.size() - lower *
		num_clusters; // these can be assigned anywhere.

	std::vector<int> assigned(num_clusters, 0);
	std::vector<int> maxima(num_clusters, lower);
	std::vector<bool> cands_assigned(penalties.size(), false);
	es_sorter sorting;
	sorting.full = std::vector<bool>(num_clusters, false);
	int error = 0;

	// Sort by reference, by minimal error.
	// Maybe do faster with ints. Later.
	std::vector<std::vector<std::vector<int> >::const_iterator> queue(
		penalties.size());

	size_t counter = 0;

	for (counter = 0; counter < penalties.size(); ++counter) {
		queue[counter] = penalties.begin() + counter;
	}

	sort(queue.begin(), queue.end(), sorting);

	for (size_t idx = 0; idx < penalties.size(); ++idx) {
		// We want to count backwards. Why not put this in the for loop?
		// Because then the end condition would be "counter >= 0" which
		// is always true!
		counter = penalties.size() - 1 - idx;

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
		for (std::vector<int>::const_iterator pos = queue[counter]->begin();
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