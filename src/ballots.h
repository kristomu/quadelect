#pragma once

#include <assert.h>

#include <boost/container/flat_set.hpp>

#include <unordered_map>
#include <stdexcept>
#include <vector>
#include <string>
#include <math.h>
#include <list>
#include <set>
#include <map>


// Ballot components. An ordering is just that, but it also includes ratings
// (if so declared), so that loser-elimination/average-elimination can work,
// and so that we may experiment with cardinal ratings later.

class candscore {

	private:
		double score;
		size_t candidate_number;

	public:
		// Do this by the book
		double get_score() const {
			return (score);
		}
		void set_score(double score_in);
		size_t get_candidate_num() const {
			return (candidate_number);
		}
		void set_candidate_num(size_t cand_in);

		candscore(size_t candnum_in);
		candscore(size_t cn_in, double score_in);

		// DONE: Trick this so that candscore a == candscore b if
		// candidate numbers are equal - to prevent double scores
		// in rankings/ratings.
		// TODO: Handle transitivity failure.
		bool operator<(const candscore & other) const {
			if (this == &other || *this == other) {
				return (false);
			}

			if (score != other.score) {
				return (score < other.score);
			} else	return (candidate_number < other.
						candidate_number);
		}

		bool operator==(const candscore & other) const {
			return (candidate_number == other.candidate_number);
		}

		bool operator>(const candscore & other) const {
			return (!(*this < other) && !(*this == other));
		}

};

typedef boost::container::flat_set<candscore, std::greater<candscore> >
ordering;

// The first is full, second is winner only.
typedef std::pair<ordering, ordering> cache_orderings;

// Does this doom all our matrices to use doubles? Seems like it - the problem
// is that we need doubles for reweighted votes, but all other times, ints
// would suffice.
class ballot_group {
	private:
		double weight;

	public:
		ordering contents;
		// If !is_complete, then the ballot isn't complete. If !rated,
		// then don't pay attention to the actual scores, just the
		// ranks.
		bool complete, rated;

		ballot_group();
		ballot_group(double weight_in);
		ballot_group(double weight_in, const ordering & cont,
			bool complete_in, bool rated_in);

		bool operator<(const ballot_group & other) const {
			if (contents != other.contents) {
				return contents < other.contents;
			}

			return weight < other.weight;
		}

		double get_weight() const {
			return weight;
		}
		void set_weight(double weight_in) {
			if (weight_in <= 0) {
				throw std::out_of_range("Ballot weight must be positive.");
			}
			weight = weight_in;
		}

		bool operator==(const ballot_group & other) const {
			return (weight == other.weight && contents == other.contents);
		}

		bool operator!=(const ballot_group & other) const {
			return (!(*this == other));
		}

		double get_max_score() const {
			return contents.begin()->get_score();
		}

		double get_min_score() const {
			return contents.rbegin()->get_score();
		}

		candscore get_candidate(size_t candidate_number) const {
			for (candscore cs: contents) {
				if (cs.get_candidate_num() == candidate_number) {
					return cs;
				}
			}

			throw std::logic_error("get_candidate: Could not find "
				"candidate!");
		}

		void replace_score(candscore old_candscore, double new_score) {
			// Remove the old candscore...
			contents.erase(old_candscore);
			// Set its score and reinsert.
			old_candscore.set_score(new_score);
			contents.insert(old_candscore);
		}

		void replace_score(size_t candidate_number, double new_score) {
			candscore this_cddt(-1, -1);
			bool found_candidate = false;
			for (candscore cs: contents) {
				if (cs.get_candidate_num() == candidate_number) {
					found_candidate = true;
					this_cddt = cs;
				}
			}
			if (!found_candidate) {
				throw std::logic_error("replace_score: Could not find "
					"candidate!");
			}

			replace_score(this_cddt, new_score);
		}
};

typedef std::list<ballot_group> election_t;