
#ifndef _VOTE_MULTIW
#define _VOTE_MULTIW

#include <ext/numeric>
#include <algorithm>
#include <iterator>
#include <vector>
#include <list>
#include <set>

#include "../ballots.cc"
#include "../positional.cc"

using namespace std;

// Maybe implement: IS_DETERMINISTIC, returns either -1 (random), 0 (determ. except
// with ties) or 1 (fully determ - presumably non-neutral).

// Majoritarian council
// Vote-based reweighted council
// Quota-based reweighted with fixed elimination order (can use any method
// for either)
// Quota-based reweighted with fluid elimination order (STV, QLTD-PR, QPQ, ...)

// How are we going to handle ties here? A list of lists could do it, with
// every possible tie, but in the case of a complete tie, that returns
// exponential numbers of lists. Perhaps a "was_tie" boolean? Hm.

class multiwinner_method {

	public:
		virtual list<int> get_council(int council_size,
			int num_candidates,
			const list<ballot_group> & ballots) const = 0;

		virtual string name() const = 0;

		// Hack?
		virtual bool polytime() const {
			return (true);
		}

};

class majoritarian_council : public multiwinner_method {
	private:
		election_method * base;

	public:
		list<int> get_council(int council_size, int num_candidates,
			const list<ballot_group> & ballots) const;

		string name() const {
			return ("Maj[" + base->name() + "]");
		}

		majoritarian_council(election_method * base_in) {
			base = base_in;
		}

};

list<int> majoritarian_council::get_council(int council_size,
	int num_candidates, const list<ballot_group> & ballots) const {

	// First get the ordering

	ordering social_order = base->elect(ballots, num_candidates);

	// Then pick the first n (note that this, strictly speaking, fails
	// neutrality. Fix later) and return it.

	// Possible way of fixing neutrality. For each level (equal rank set),
	// acquire those that are ranked at that level, shuffle the result,
	// and read off until the council has been filled.

	list<int> council;

	int count = 0;

	for (ordering::const_iterator pos = social_order.begin(); pos !=
		social_order.end() && count < council_size; ++pos) {
		council.push_back(pos->get_candidate_num());
		++count;
	}

	return (council);
}

class random_council : public multiwinner_method {
	public:
		list<int> get_council(int council_size, int num_candidates,
			const list<ballot_group> & ballots) const;

		string name() const {
			return ("Random council");
		}

};

list<int> random_council::get_council(int council_size, int num_candidates,
	const list<ballot_group> & ballots) const {

	// First generate a candidate list 0..num_candidates. Then shuffle
	// them randomly and pick the first council_size number.

	// This could be cached.
	vector<int> numbered_candidates(num_candidates);
	iota(numbered_candidates.begin(), numbered_candidates.end(), 0);

	random_shuffle(numbered_candidates.begin(), numbered_candidates.end());

	list<int> toRet;

	copy(numbered_candidates.begin(), numbered_candidates.begin() +
		council_size, inserter(toRet, toRet.begin()));

	return (toRet);
}

// --- //

// Multiplicative voter-based reweighting.
// 	WeightNew = WeightOld * A / (B + curscore / maxscore)
// 	C / (/*C +*/ inner_denom)
class mult_ballot_reweighting : public multiwinner_method {

	private:
		positional * base; // But not Bucklin or QLTD
		double A, B;

	public:
		list<int> get_council(int council_size, int num_candidates,
			const list<ballot_group> & ballots) const;

		string name() const {
			return ("ReweightM[" +base->name() + "]");
		}

		mult_ballot_reweighting(positional * base_in) {
			base = base_in; A = 0.5; B = 1;
		}
		mult_ballot_reweighting(positional * base_in, double A_in,
			double B_in) {
			base = base_in; A = A_in;
			B = B_in;
		}

};

list<int> mult_ballot_reweighting::get_council(int council_size,
	int num_candidates, const list<ballot_group> & ballots) const {

	// first get the social ordering for the entire ballot set. Once that's
	// done, pick the unelected candidate that's closest to the winner (and
	// may often be the winner outright). Then, for each voter, get that
	// candidate's score based on the voter's ballot alone, and reweight by
	// C / (C + relative power), where relative power is how many points he
	// contributed divided by the maximum number of points attainable.

	// Since determining the social order is relatively expensive for a
	// positional system when candidates >> voters, we need a separate
	// function or heuristic for this so that it just calculates the score
	// directly in that case. (We do the former, should do the latter,
	// though.)

	list<int> council;
	int count = 0;

	list<ballot_group> reweighted_ballots = ballots;

	vector<bool> elected(num_candidates, false);

	while (count < council_size) {

		ordering social_order = base->elect(reweighted_ballots,
				num_candidates);

		// Get the winner.
		int next = -1;
		for (ordering::const_iterator pos = social_order.begin(); pos !=
			social_order.end() && next == -1; ++pos)
			if (!elected[pos->get_candidate_num()]) {
				next = pos->get_candidate_num();
			}

		assert(next != -1);

		// Elect him.
		council.push_back(next);
		elected[next] = true;
		++count;

		if (count == council_size) {
			continue;
		}

		// Reweight the voters according to their contribution to
		// the person's victory. (Perhaps a more advanced mechanism
		// would use quota and surplus instead, in two steps).
		// 	But that'd require elimination, at which point we have
		// 	STV.

		// Should this be "num_unelecteds" ?
		double maxscore = base->get_pos_maximum(num_candidates);

		if (maxscore == 0) {
			maxscore = 1e-6;
		}

		double C = 0.5;

		// TODO: Handle fractional votes, since they only count
		// (value / number of equally ranked votes) and so gps should
		// be divided by (number of equally ranked votes) in that case.
		for (list<ballot_group>::iterator ballot_pos =
				reweighted_ballots.begin(); ballot_pos !=
			reweighted_ballots.end(); ++ballot_pos) {
			double current_state = base->get_pos_score(
					*ballot_pos, next, num_candidates);
			double inner_denom = B + (current_state / maxscore);

			if (inner_denom == 0) {
				continue;
			}
			assert(inner_denom > 0);

			//cout << "Debug " << name() << ": " << current_state << "\t" << inner_denom << "\t" << C / inner_denom << "\t" << ballot_pos->weight << " -> ";

			ballot_pos->weight *= A / (/*C +*/ inner_denom);
			//cout << ballot_pos->weight << endl;
		}
	}

	return (council);
}

// Additive voter-base reweighting.
// Say the original weight is origwt. Then at some round,
// 	weight = origwt * C / (C + Q)
// 	where
// 		C = M (D'Hondt) or M/2 (Sainte-Lague)
// 		M = maximum attainable score of all the rounds
// 		Q = sum of score given to elected candidates
// There are two ways this can be treated. Either M is the same for all rounds
// for positional systems, or it's the maximum given the number of unelected
// candidates left. The latter seems to give better scores. For Range, M is
// fixed and so the two approaches are equal.
class addt_ballot_reweighting : public multiwinner_method {

	private:
		positional * base; // But not QLTD or Bucklin. But Range.

	public:
		list<int> get_council(int council_size, int num_candidates,
			const list<ballot_group> & ballots) const;

		addt_ballot_reweighting(positional * base_in) {
			base = base_in;
		}
		string name() const {
			return ("ReweightA[" +base->name() + "]");
		}
};

list<int> addt_ballot_reweighting::get_council(int council_size,
	int num_candidates, const list<ballot_group> & ballots) const {

	// First get the original weightings and count the number of ballots.

	vector<double> orig_weightings;

	list<ballot_group>::const_iterator qpos;
	list<ballot_group>::iterator wpos;

	for (qpos = ballots.begin(); qpos != ballots.end(); ++qpos) {
		orig_weightings.push_back(qpos->weight);
	}

	int num_ballots = orig_weightings.size();

	vector<double> points_attributed(num_ballots, 0),
		   point_maximum(num_ballots, 0);

	list<int> council;
	int council_count = 0;

	vector<bool> elected(num_candidates, 0);

	list<ballot_group> reweighted_ballots = ballots;

	while (council_count < council_size) {
		// Calculate who wins this round.

		ordering social_order = base->elect(reweighted_ballots,
				num_candidates);

		int next = -1;
		for (ordering::const_iterator opos = social_order.begin();
			opos != social_order.end() && next == -1;
			++opos)
			if (!elected[opos->get_candidate_num()]) {
				next = opos->get_candidate_num();
			}

		// Set this guy to elected, and update the weight counts

		assert(next != -1);

		elected[next] = true;
		council.push_back(next);
		++council_count;
		if (council_count == council_size) {
			continue;
		}

		double maxscore = base->get_pos_maximum(num_candidates - council_count);
		int counter = 0;

		for (wpos = reweighted_ballots.begin(); wpos !=
			reweighted_ballots.end(); ++wpos) {
			points_attributed[counter] += base->get_pos_score(
					*wpos, next, num_candidates);
			point_maximum[counter] += maxscore;

			double C = point_maximum[counter] * 0.5;

			double denom = C + points_attributed[counter];

			if (denom == 0) {
				continue;
			}

			wpos->weight = orig_weightings[counter] * C / (C +
					points_attributed[counter]);

			++counter;
		}
	}

	return (council);
}


#endif
