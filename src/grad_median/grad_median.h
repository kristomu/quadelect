// Data structure for "gradual median", which is a gradual relaxation of the
// median from median at one end and the mean (or total sum) at another. When 
// all ballot weights are equal, the gradual relaxation is equivalent to 
// including the element above and below the current rating for each step, thus
// relaxing from median to sum.

// The entire point of this data structure is to handle the case where the
// ballot weights aren't equal. To do that, we use a plane-sweep algorithm where
// two lines starting at the middle of the sorted ratings function, thus 
// corresponding to the median, are moved outwards, thus including more and more
// of the function. This line stops at each event that could change a given
// ratings function's truncated mean ratings, and updates it at that point.

// To use the structure, first input the required ratings and run the init
// function with the desired center (0.5 for median, 0 for lowest, 1 for 
// highest). Then call update(), which will return true if something changed,
// false otherwise, and if necessary, exclude(candidate), which will ignore that
// candidate in the future.

// The structure is used for VMedian and for gradual Black / set narrowing 
// gradual Black. It is polyspace summable if all ratings are integer and
// the number of distinct ratings permitted is independent of the number of
// voters.

// Note that update changes data in the structure itself, so it cannot be
// const.

// [TODO: Explain different approaches of completing incomplete ballots, and
//  show that completion by one less than minimum is that which would minimize
//  no-show paradox problems.]

#ifndef _VOTE_GMEDIAN
#define _VOTE_GMEDIAN

#include <algorithm>
#include <iterator>

#include <assert.h>
#include <math.h>

#include <iostream>
#include <vector>
#include <queue>
#include <list>

using namespace std;

class priority_entry {
	public:
		double distance, score;
		int array;

		priority_entry(double dist, int array_in, double score_in) {
			distance = dist; array = array_in; score = score_in; }
		// Empty, so it'll err in valgrind if misused.
		priority_entry() {}

		// The priority entry is used in a queue that sorts by distance.
		bool operator > (const priority_entry & other) const {
			return(distance > other.distance);
		}
};

enum completion_type { GF_GREATEST, GF_LEAST, GF_BOTH, GF_NONE };

class grad_fracile {

	private:
		vector<list<pair<double, double> > > sorted_lists;
		vector<bool> hopefuls;
		vector<double> scores;
		bool does_need_reinit, all_sorted, changed;
		size_t num_hopefuls;
		completion_type completion;

		list<pair<double, double> > minscorev, maxscorev;
		vector<list<pair<double, double> > > finite_min_sentinels,
			finite_max_sentinels;

		priority_queue<priority_entry, vector<priority_entry>,
			greater<priority_entry> > left_pq, right_pq;
		vector<list<pair<double, double> >::const_iterator >
			left_indices, right_indices;

		double get_center(list<pair<double, double> >::const_iterator &
				lower, list<pair<double, double> >::
				const_iterator & upper, 
				const list<pair<double, double > > & source, 
				double numvoters, double fracile) const;

		void add_score(size_t candidate, double left, double right);

		// Required for summability
		void compress_candidate(list<pair<double, double> > & 
				cand_scores, bool sorted);

	public:
		grad_fracile();

		size_t get_num_candidates() const { return(sorted_lists.size()); }
		int add_candidate(); // returns #cands
		int add_candidates(int how_many);
		int remove_last_candidate(); // returns #cands.

		bool add_rating(size_t candidate, double weight, double rating);
		bool set_rating(size_t candidate, double weight, double rating);

		bool exclude(size_t candidate);
		bool include(size_t candidate);
		bool is_hopeful(size_t candidate) const;
		int get_num_hopefuls() const;
		void reset_hopefuls(); 		// Make all included again.

		double get_score(size_t candidate) const;
		// Linear time!
		double lget_eff_voters(size_t candidate) const;

		bool needs_reinit() const { return(does_need_reinit); }
		bool init(double fracile, completion_type completion_in);

		// Returns true if ties were broken in the last update.
		// Note that cardinal values (scores) can change with this
		// still being false.
		bool ordinal_changed() const;

		// Used for debugging.
		void print_next_pair(size_t candidate, bool left) const;

		// Returns true if the update went alright, otherwise false.
		// False may be error or it may be that there are no candidates
		// or nothing left to update (sweep line is past the data).
		bool update(bool debug);

		void compress();
};

#endif
