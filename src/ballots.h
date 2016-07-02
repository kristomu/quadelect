#ifndef _VOTE_BALLOTS
#define _VOTE_BALLOTS

#include <assert.h>

#include <unordered_map>
#include <vector>
#include <string>
#include <math.h>
#include <list>
#include <set>
#include <map>

using namespace std;

// Ballot components. An ordering is just that, but it also includes ratings
// (if so declared), so that loser-elimination/average-elimination can work,
// and so that we may experiment with cardinal ratings later.

class candscore {

	private:
		double score;
		int candidate_number;

	public:
		// Do this by the book
		double get_score() const { return(score); }
		void set_score(double score_in);
		int get_candidate_num() const { return(candidate_number); }
		void set_candidate_num(int cand_in);

		candscore(int candnum_in);
		candscore(int cn_in, double score_in);

		// DONE: Trick this so that candscore a == candscore b if
		// candidate numbers are equal - to prevent double scores
		// in rankings/ratings.
		// TODO: Handle transitivity failure.
		bool operator<(const candscore & other) const {
			if (this == &other || *this == other)
				return (false);

			if (score != other.score)
				return (score < other.score);
			else	return (candidate_number < other.
					candidate_number);
		}

		bool operator==(const candscore & other) const {
			return(candidate_number == other.candidate_number);
		}

		bool operator>(const candscore & other) const {
			return (!(*this < other) && !(*this == other));
		}

};

typedef set<candscore, greater<candscore> > ordering;

typedef pair<ordering, ordering> cache_orderings; // first is full, second is winner only.

// Does this doom all our matrices to use doubles? Seems like it - the problem
// is that we need doubles for reweighted votes, but all other times, ints
// would suffice. 
class ballot_group {
	public:
		double weight;
		ordering contents;
		// If !is_complete, then the ballot isn't complete. If !rated,
		// then don't pay attention to the actual scores, just the
		// ranks.
		bool complete, rated;

		ballot_group();
		ballot_group(double weight_in);
		ballot_group(double weight_in, const ordering & cont, 
				bool complete_in, bool rated_in);
		~ballot_group(); // It's said that this'll prevent leaks
};

#endif
