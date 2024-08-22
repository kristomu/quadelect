// Hard Cardinal-type voting methods: Birational voting and LPV.
// These do an exhaustive search over the candidate space, and so can take a
// VERY long time for many candidates.

// Perhaps include minimax.

#ifndef _VOTE_HARDCARD
#define _VOTE_HARDCARD

#include "methods.cc"
#include <list>
#include <vector>

using namespace std;

// Auxiliary functions. TESTS thus KLUDGY. Fix later

double gini_one(const vector<double> & a) {

	double total = 0;
	double r_gini_count = 0;

	for (int counter = 0; counter < a.size(); ++counter) {
		for (int sec = 0; sec < a.size(); ++sec) {
			r_gini_count += fabs(a[counter] - a[sec]);
		}
		total += a[counter];
	}

	double mean = total / (double)a.size();

	double gini = r_gini_count / (2 * a.size() * a.size() * mean);

	return (gini);
}

// Sen's welfare function
double welfare(const vector<double> & a) {

	double total = 0;

	for (int counter = 0; counter < a.size(); ++counter) {
		total += a[counter];
	}

	return ((1-gini_one(a)) * (total / (double)(a.size())));
}

/*typedef enum hardcard_type { HC_BIRATIONAL, HC_LPV };

typedef struct cardinal_ballot {
	        double weight; // #voters
		        vector<double> scores; // #voters by #cands
};

typedef struct hardcard_extrema {
	        vector<bool> W_minimum_set, W_maximum_set;
		        double W_minimum, W_maximum;
};


double wfa(const vector<bool> & W, const vector<cardinal_ballot> &
		ballots) {

	vector<double> scores;

	for (int counter = 0; counter < ballots.size(); ++counter) {
		for (int sec = 0; sec < W.size(); ++sec) {
			if (!W[sec]) continue;

			scores.push_back(ballots[counter].weight *
					ballots[counter].scores[sec]);
		}
	}

	return(welfare(scores));
}*/


/////////////////////////////////////

typedef enum hardcard_type { HC_BIRATIONAL, HC_LPV };

typedef struct cardinal_ballot {
	double weight; // #voters
	vector<double> scores; // #voters by #cands
};

typedef struct hardcard_extrema {
	vector<bool> W_minimum_set, W_maximum_set;
	double W_minimum, W_maximum;
};

class hardcard : public multiwinner_method {

	private:
		hardcard_type type;

		vector<cardinal_ballot> make_cardinal_array(
			const list<ballot_group> & ballots, int
			numcand) const;

		double birational(const vector<bool> & W, const
			cardinal_ballot & this_ballot) const;

		double birational(const vector<bool> & W, const vector<
			cardinal_ballot> & ballots) const;

		hardcard_extrema merge_extrema(const hardcard_extrema a,
			const hardcard_extrema b) const;

		hardcard_extrema all_birational(const vector<cardinal_ballot> &
			ballots, const vector<bool> & cur_W,
			vector<bool>::iterator begin,
			vector<bool>::iterator end,
			int marks_left) const;

		double LPV(const vector<bool> & W, int council_size,
			const cardinal_ballot &
			this_ballot, double k) const;

		double LPV(const vector<bool> & W, int council_size,
			const vector<cardinal_ballot>
			& ballots, double k) const;

		hardcard_extrema all_LPV(const vector<cardinal_ballot> &
			ballots, const vector<bool> & cur_W, double k,
			int council_size, vector<bool>::iterator begin,
			vector<bool>::iterator end,
			int marks_left) const;

	public:
		bool polytime() const {
			return (false);    // something like this
		}

		list<int> get_council(int council_size, int num_candidates,
			const list<ballot_group> & ballots) const;

		string name() const;

		hardcard(hardcard_type type_in) {
			type = type_in;
		}

};

vector<cardinal_ballot> hardcard::make_cardinal_array(const
	list<ballot_group> & ballots, int numcand) const {

	// This is used to calculate birational and LPV results quickly, as
	// those methods have terms like "voter X's rating of candidate Y".
	// With the usual ballot format, that would take at least linear
	// (perhaps quadratic) time, whereas this takes constant.

	// We use NAN for "no opinion" Range-style values.

	vector<cardinal_ballot> results(ballots.size());

	int counter = 0;
	for (list<ballot_group>::const_iterator pos = ballots.begin();
		pos != ballots.end(); ++pos) {

		results[counter].weight = pos->weight;
		results[counter].scores.resize(numcand, NAN);

		for (ordering::const_iterator opos = pos->contents.begin();
			opos != pos->contents.end(); ++opos) {
			results[counter].scores[opos->get_candidate_num()] =
				opos->get_score();
			cout << opos->get_score() << endl;
		}
	}

	return (results);
}

double hardcard::birational(const vector<bool> & W, const cardinal_ballot &
	this_ballot) const {

	//                                     x_w
	// L(w) = SUM      SUM      SUM      -------
	// 	  vote     w in W   s in W   1 + x_s
	// 	  vectors
	// 	  x->

	// We don't handle Range-style ballots yet - they get set to 0.
	// Possible later TODO, change 1 + x_s so that the D'Hondt
	// generalization of PAV turns into Sainte-Laguë instead.

	double outer_count = 0, inner_count = 0;
	double total = 0;

	// Minimax doesn't do much better either.

	for (int w = 0; w < W.size(); ++w) {
		if (!W[w]) {
			continue;
		}

		for (int s = 0; s < W.size(); ++s) {
			if (!W[s]) {
				continue;
			}
			double atw = this_ballot.scores[w],
				   ats = this_ballot.scores[s];

			if (!finite(atw)) {
				atw = 0;
			}
			if (!finite(ats)) {
				ats = 0;
			}

			total += atw / (1 + ats);
		}
	}

	return (total * this_ballot.weight);
}

double hardcard::birational(const vector<bool> & W,
	const vector<cardinal_ballot> & ballots) const {

	/*double toRet = wfa(W, ballots);

	cout << "Return val: " << toRet << endl;

	return(toRet);

	assert (1 != 1);*/

	double total = 0;

	for (vector<cardinal_ballot>::const_iterator pos = ballots.begin();
		pos != ballots.end(); ++pos) {
		total += birational(W, *pos);
	}

	return (total);
}


// For merging recursion values.
hardcard_extrema hardcard::merge_extrema(const hardcard_extrema a,
	const hardcard_extrema b) const {

	hardcard_extrema toRet = a;

	/*vector<bool> W_minimum_set, W_maximum_set;
	double W_minimum, W_maximum;*/

	if (b.W_minimum < toRet.W_minimum || isnan(toRet.W_minimum)) {
		toRet.W_minimum = b.W_minimum;
		toRet.W_minimum_set = b.W_minimum_set;
	}

	if (b.W_maximum > toRet.W_maximum || isnan(toRet.W_maximum)) {
		toRet.W_maximum = b.W_maximum;
		toRet.W_maximum_set = b.W_maximum_set;
	}

	return (toRet);
}

// Ain't recursion nifty? Here we recurse to find the best of all possible
// council sets.
hardcard_extrema hardcard::all_birational(const vector<cardinal_ballot> &
	ballots, const vector<bool> & cur_W, vector<bool>::iterator
	begin, vector<bool>::iterator end, int marks_left) const {

	// All marked!
	if (marks_left == 0) {
		hardcard_extrema result;
		result.W_minimum_set = cur_W;
		result.W_minimum = birational(cur_W, ballots);

		result.W_maximum_set = result.W_minimum_set;
		result.W_maximum = result.W_minimum;

		return (result);
	}

	// Not an admissible council since marks_left != 0
	if (begin == end) {
		hardcard_extrema result;
		result.W_minimum = NAN;
		result.W_maximum = NAN;
		return (result);
	}

	// Not at the end, but marks_left isn't 0 either! So recurse twice,
	// one towards the end with cur_W not set, another towards the end with
	// cur_W set and marks_left--.

	*begin = true;
	hardcard_extrema a = all_birational(ballots, cur_W, begin+1, end,
			marks_left - 1);
	*begin = false;
	// TODO later: early break, if even if all later were true, marks
	// left would be above zero.
	hardcard_extrema b = all_birational(ballots, cur_W, begin+1, end,
			marks_left);

	return (merge_extrema(a, b));
}

double hardcard::LPV(const vector<bool> & W, int council_size,
	const cardinal_ballot & this_ballot, double k) const {

	//                                      /     K + |W|    \
	// L_k(W) = SUM     SUM       x_j *   ln| -------------- |
	//          vote    j in C              | K + SUM    x_s |
	//          vector                      \     s in W     /

	// That can be optimized by calculating the logarithm just once,
	// which we do.

	// Get denominator
	double log_denom = k;

	int counter = 0;

	for (counter = 0; counter < W.size(); ++counter)
		if (W[counter]) {
			log_denom += this_ballot.scores[counter];
		}

	cout << "LPV: Log denom is " << log_denom << endl;

	double logarithm = log((k + council_size)/(log_denom));

	if (!finite(logarithm)) {
		return (logarithm);
	}

	double total = 0;

	for (counter = 0; counter < this_ballot.scores.size(); ++counter) {
		total += (this_ballot.scores[counter] * logarithm);
	}

	return (this_ballot.weight * total);
}


double hardcard::LPV(const vector<bool> & W, int council_size,
	const vector<cardinal_ballot> & ballots, double k) const {

	cout << "Entering LPV" << endl;

	double total = 0;

	for (int counter = 0; counter < ballots.size() && finite(total);
		++counter) {
		total += LPV(W, council_size, ballots[counter], k);
	}

	return (total);
}

hardcard_extrema hardcard::all_LPV(const vector<cardinal_ballot> & ballots,
	const vector<bool> & cur_W, double k, int council_size,
	vector<bool>::iterator begin,
	vector<bool>::iterator end, int marks_left) const {

	// See birational.
	hardcard_extrema result;

	if (marks_left == 0) {
		/*int count = 0;
		for (int y = 0; y < cur_W.size(); ++y) {
			if (cur_W[y]) ++count;
		}

		cout << "marksleft 0: count: " << count << " vs " << council_size << "\t";*/
		result.W_minimum_set = cur_W;
		result.W_minimum = LPV(cur_W, council_size, ballots, k);
		cout << "result val:" << result.W_minimum << endl;

		result.W_maximum_set = result.W_minimum_set;
		result.W_maximum = result.W_minimum;
		return (result);
	}

	if (begin == end) {
		result.W_minimum = NAN;
		result.W_maximum = NAN;
		return (result);
	}

	*begin = true;
	hardcard_extrema a = all_LPV(ballots, cur_W, k, council_size, begin+1,
			end, marks_left - 1);
	*begin = false;
	hardcard_extrema b = all_LPV(ballots, cur_W, k, council_size, begin+1,
			end, marks_left);

	return (merge_extrema(a, b));
}

list<int> hardcard::get_council(int council_size, int num_candidates,
	const list<ballot_group> & ballots) const {

	vector<cardinal_ballot> cballots = make_cardinal_array(ballots,
			num_candidates);
	vector<bool> W(num_candidates, false);

	list<int> council;
	int counter;

	if (type == HC_BIRATIONAL) {
		hardcard_extrema birat = all_birational(cballots, W, W.begin(),
				W.end(), council_size);

		// Turn into council
		vector<bool> winner_b = birat.W_maximum_set;

		for (counter = 0; counter < winner_b.size(); ++counter)
			if (winner_b[counter]) {
				council.push_back(counter);
			}
	}

	if (type == HC_LPV) {
		double k = 1e-13; // ideally 0, but that brings infinities

		hardcard_extrema result = all_LPV(cballots, W, k, council_size,
				W.begin(), W.end(), council_size);

		for (counter = 0; counter < result.W_minimum_set.size();
			++counter)
			if (result.W_minimum_set[counter]) {
				council.push_back(counter);
			}

		cout << "LPV: Council size: " << council.size() << "and score " <<
			result.W_minimum << endl;
	}

	return (council);
}

string hardcard::name() const {
	switch (type) {
		case HC_LPV: return ("Cardinal: LPV");
		case HC_BIRATIONAL: return ("Cardinal: Birational");
	}

	assert(1 != 1);
}

#endif
