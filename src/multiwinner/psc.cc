#ifndef _VOTE_PSC
#define _VOTE_PSC

#include "../condorcet/methods.cc"
#include "../ballots.cc"
#include "../dsc.cc"
#include "methods.cc"
#include <list>

// TODO: User can spec. DSC, DAC, and HDSC. Makes no difference yet because
// all ballots are complete.

// TODO: Figure out why the map of sets become so extremely large. After all,
// it shouldn't be of greater size than (1 + 2 + 3 + 4 + .. n) * n. But
// the above is 0.5n^2, thus we have n^3, which could explain it. A trie could
// do better.

class PSC : public multiwinner_method {

	private:
		list<int> get_council(double C, double divisor, int council_size,
			int num_candidates, const list<ballot_group> &
			ballots) const;

	public:
		list<int> get_council(int council_size, int num_candidates,
			const list<ballot_group> & ballots) const;

		string name() const {
			return ("PSC-CLE");
		}

};

// Returns -1 if we exceeded the council size, -2 if we were short of it.
// Divisor may be nonmonotonic - check later. Not excluding that which was
// elected makes it monotonic, check that too later.

list<int> PSC::get_council(double C, double divisor, int council_size,
	int num_candidates, const list<ballot_group> & ballots) const {

	list<int> too_few, too_many;
	too_few.push_back(-2);
	too_many.push_back(-1);

	// First, generate the DSC multimap. Then divide the doubles of each
	// by the quota, which may be Droop or whatever you want. (Divisor
	// methods could also be done by trial and failure, e.g (num_voters / p)
	// where p is set so that the number in the council turns out just
	// right.

	// Once that's done, then generate the fallback preference ordering
	// (single-winner Condorcet or whatever). The main loop:
	// 	If there's a set where the number of members >= number that can
	// 	be elected (according to quota), then elect the members and set
	// 	those as "to be excluded".
	// 	Otherwise, set the non-excluded member that's lowest ranked on
	// 		the FPO to be excluded.
	//
	// 	Then remove those set as "to be excluded". This is done by
	// 	subtracting them from all the sets in the multimap.

	// As it is, the method isn't competitive, but it might be salvagable
	// with some divisor-type tweaks.

	cout << "One" << endl;
	multimap<double, set<unsigned short> > init_coalitions =
		get_dsc(ballots), coalitions;

	cout << "Two" << endl;

	/*list<ballot_group>::const_iterator lpos;
	double num_voters = 0;
	for (lpos = ballots.begin(); lpos != ballots.end(); ++lpos)
		num_voters += lpos->weight;*/

	// Droop quota. See stv.cc
	//double quota = num_voters / (double)(council_size + 1);

	// Check Schulze later. Now uses Schulze, gives a slight improvement.
	ordering fallback = schulze(CM_WV).cond_elect(condmat(ballots,
				num_candidates, CM_WV, false));

	int num_elected = 0;
	vector<bool> hopeful(num_candidates, true);
	set<int> council;

	multimap<double, set<unsigned short> >::iterator apos =
		init_coalitions.begin(), old_apos;

	cout << "Three" << endl;

	// Normalize to quota. The PSC-CLE spec says to round down, but we
	// might try rounding off, later.
	while (apos != init_coalitions.end()) {
		//double adj_quota = round((apos->first / quota));
		double adj_quota = floor((apos->first / divisor) + C);

		// No point in adding coalitions that can never get elected.
		if (adj_quota > 0)
			coalitions.insert(pair<double, set<unsigned short> >(
					adj_quota, apos->second));

		// Yuck, but keeps the memory down.
		old_apos = apos;
		++apos;
		init_coalitions.erase(old_apos);
	}

	cout << "Four" << endl;

	while (council.size() < council_size) {

		multimap<double, set<unsigned short> >::const_iterator pos;
		set<unsigned short> excludable, electable;

		bool once_through = false;

		int oldsize = council.size();

		for (pos = coalitions.begin(); pos != coalitions.end() &&
			council.size() < council_size;
			++pos) {
			//cout << "Checking coalition of size " << pos->second.size() << " with " << pos->first << " quotas permitted." << endl;
			// If this coalition is admissible, admit it.
			if (pos->second.size() == pos->first &&
				!pos->second.empty()) {
				// halting here after one pass through is BAD.
				//cout << "Coalition picked." << endl;
				copy(pos->second.begin(), pos->second.end(),
					inserter(council,
						council.begin()));
				copy(pos->second.begin(), pos->second.end(),
					inserter(electable,
						electable.begin()));
				/*copy(pos->second.begin(), pos->second.end(),
						inserter(excludable,
							excludable.begin()));*/

			}
		}

		cout << "Council size is now " << council.size() << " where we want " <<
			council_size << endl;

		// If we have them all, no need to exclude any more; it'll hop
		// off the loop at the next go.
		if (council.size() == council_size) {
			continue;
		}

		// If the excludable set is empty (i.e, we found no coalition
		// deserving a seat), then exclude the one that's lowest in the
		// fallback order.

		if (council.size() == oldsize) {
			int elim_next = -1;
			// WARNING: Random tiebreak!
			// Maybe first skip (both eliminated and elected)
			// and if that doesn't work, skip (eliminated only).
			for (ordering::const_reverse_iterator rp = fallback.
					rbegin(); rp != fallback.rend() &&
				elim_next == -1; ++rp)
				if (hopeful[rp->get_candidate_num()]) {
					elim_next = rp->get_candidate_num();
				}

			if (elim_next == -1) {
				return (too_few);
			}
			//assert(elim_next != -1);

			excludable.insert(elim_next);
		}

		// Okay, first remove the excludables from the coalition sets
		// that remain.
		apos = coalitions.begin();
		while (apos != coalitions.end()) {

			set<unsigned short> dest;

			// First, remove the excluded members.
			set_difference(apos->second.begin(), apos->second.end(),
				excludable.begin(), excludable.end(),
				inserter(dest, dest.begin()));

			// Then copy the new set over the old one.
			if (!dest.empty()) {
				apos->second = dest;
				++apos;
			} else {
				old_apos = apos;
				++apos;
				coalitions.erase(old_apos);
			}
		}

		// Then mark all of these as excluded.
		set<unsigned short>::const_iterator xpos;

		for (xpos = excludable.begin(); xpos != excludable.end(); ++xpos) {
			hopeful[*xpos] = false;
		}
		/*	for (xpos = electable.begin(); xpos != electable.end(); ++xpos)
				hopeful[*xpos] = false;*/
	}

	// At this point, council is a set. We want a list, so just copy it in
	// and return.

	if (council.size() > council_size) {
		return (too_many);
	}
	if (council.size() < council_size) {
		return (too_few);
	}

	list<int> elected_council;
	copy(council.begin(), council.end(), inserter(elected_council,
			elected_council.begin()));

	/* UGLY HACK
	elected_council.resize(min(elected_council.size(), (size_t)council_size));*/

	return (elected_council);
}

list<int> PSC::get_council(int council_size, int num_candidates,
	const list<ballot_group> & ballots) const {

	// Start off by divisor = num_voters / council_size (Hare).
	// Then, by binary search, adjust upwards if too many, downwards if
	// too few.

	list<ballot_group>::const_iterator lpos;
	double num_voters = 0;
	for (lpos = ballots.begin(); lpos != ballots.end(); ++lpos) {
		num_voters += lpos->weight;
	}

	// Range: Minimum: 1. Maximum: num_voters.
	// Experimental and not really a good idea.

	double low = 0, high = num_voters - 1, mid = -1;

	while (low <= high) {
		if (mid == -1) {
			mid = num_voters / (double)council_size;
		} else	{
			mid = (low + high) * 0.5;
		}

		cout << "Trying divisor " << mid << endl;

		list<int> tentative = get_council(0.5, mid, council_size,
				num_candidates, ballots);

		// Too many, thus divisor is too low.
		if (*(tentative.begin()) == -1) {
			low = mid + 0.001;
		} else {
			if (*(tentative.begin()) == -2) {
				high = mid - 0.001;
			} else {
				return (tentative);
			}
		}
	}

	assert(1 != 1);  // NOT FOUND!
}

#endif
