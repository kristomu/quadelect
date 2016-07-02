// "Mode" ratings, to complete the statistical methods.
// This method finds the candidate with the highest mode (most frequently
// occurring value). Ties are broken by a leximax procedure: if most frequently
// occurring value is the same, look at next-to-most frequent value, then next
// to next, etc.

#include "mode.h"

#include <map>
#include <iterator>

using namespace std;

pair<ordering, bool> mode_ratings::elect_inner(const list<ballot_group> & 
		papers, const vector<bool> & hopefuls,
		int num_candidates, cache_map & cache, bool winner_only) const {

	// Since we don't know the number of buckets we need in advance to find
	// the mode, use a map, as it will create new buckets if we want it.
	// Then we dump that in sorted order into a list, so that we may use
	// leximax comparisons. TODO: Make a general leximax class.

	bool debug = false;

	vector<map<double, double> > mode_maps(num_candidates);

	for (list<ballot_group>::const_iterator pos = papers.begin(); pos !=
			papers.end(); ++pos)
		for (ordering::const_iterator opos = pos->contents.begin();
				opos != pos->contents.end(); ++opos)
			if (hopefuls[opos->get_candidate_num()])
				mode_maps[opos->get_candidate_num()]
					[opos->get_score()] += pos->weight;

	// Dump into lists.

	vector<pair<list<double>, int> > candidate_scores;

	for (int counter = 0; counter < mode_maps.size(); ++counter) {

		if (debug)
			cout << "Candidate " << counter << endl;

		if (!hopefuls[counter]) continue;

		// Create a rearranged list of (frequency, number) pairs.
		// We'll sort this list so that the highest frequency comes
		// first, and then just read off it.
		// (Are ties going to be a problem?)
		list<pair<double, double> > rearranged;

		for (map<double, double>::const_iterator mpos = 
				mode_maps[counter].begin(); mpos !=
				mode_maps[counter].end(); ++mpos) {
			
			if (debug)
				cout << "\t Adding (val: " << mpos->first 
					<< " freq: " << mpos->second << ")" 
					<< endl;

			rearranged.push_back(pair<double, double>(mpos->second,
						mpos->first));
		}

		rearranged.sort(greater<pair<double, double> >());

		list<double> this_cand_score;

		for (list<pair<double, double> >::const_iterator lpos =
				rearranged.begin(); lpos != rearranged.end();
				++lpos)
			this_cand_score.push_back(lpos->second);

		if (debug) {
			cout << "Values in frequency order: ";
			copy(this_cand_score.begin(), this_cand_score.end(), 
					ostream_iterator<double>(cout, " "));
			cout << endl;
		}

		// To sort, we have to mark the list by what candidate it
		// belongs to, so moving the lists around won't make it unclear
		// who is actually at top.
		candidate_scores.push_back(pair<list<double>, int>(
					this_cand_score, counter));
	}

	// Sort in least first order.
	sort(candidate_scores.begin(), candidate_scores.end());

	// Produce an ordering from this. Where have I seen this sort of code
	// before? :p We go from lowest rank to highest so that we can start
	// at zero no matter the number of candidates.
	
	int lincount = 0, last_shown = 0;

	vector<pair<list<double>, int> >::const_iterator vpos, prev;
	ordering out;

	for (vpos = candidate_scores.begin(); vpos != candidate_scores.end();
			++vpos) {

		if (debug) {
			cout << "Assigning ranks: Candidate " << vpos->second 
				<< " has these values: ";
			copy(vpos->first.begin(), vpos->first.end(), 
					ostream_iterator<double>(cout, " "));
			cout << endl;
		}

		// If it's the first (in which case it doesn't matter what we
		// label it as long as the others are of higher rank), or if
		// it's different from the previous once, update last_show so
		// that it will get a different rank.
		if (vpos == candidate_scores.begin() || vpos->first > 
				prev->first)
			last_shown = lincount;

		if (debug)
			cout << "\tThe candidate thus gets rank " << last_shown
				<< endl;

		out.insert(candscore(vpos->second, last_shown));
		++lincount;
		prev = vpos;
	}

	return(pair<ordering, bool>(out, false));
}
