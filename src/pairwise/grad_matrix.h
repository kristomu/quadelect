// Gradual Condorcet-Borda matrix. Each entry starts by being the pair's median
// rank position difference. If there is a Condorcet winner, every such pair
// involving the CW on the winning side will have a positive value, and every
// pair involving the CW on the losing side will have a negative value.

// The matrix then has an "update" method which expands the truncated mean from
// the median out towards the mean. When it becomes the mean of all values, the
// Borda winner will beat all the others. Thus, the matrix lets one combine
// Borda and Condorcet into a method that will give as little as possible in
// the direction of Borda as required to break cycles or ties.

#pragma once

#include "matrix.h"
#include "grad_median/grad_median.h"

class cond_borda_matrix : public abstract_condmat {

	private:
		grad_fracile engine;
		size_t num_candidates;
		size_t linear(size_t cand, size_t against, size_t numcands) const;

	protected:
		// Aborts if not prepared. Remember to reinit after all
		// set/add!
		double get_internal(size_t candidate, size_t against,
			bool raw) const;

		// add_internal adds the specified (weight, value) vote to the
		// gradual relaxation curve that we draw the truncated median
		// from. set_internal does the same, but clears what was there
		// before.

		bool add_internal(size_t candidate, size_t against, double weight,
			double value);
		bool set_internal(size_t candidate, size_t against, double weight,
			double value);

		bool set_internal(size_t candidate, size_t against, double value);

	public:

		cond_borda_matrix(pairwise_type type_in) :
			abstract_condmat(type_in) {}
		cond_borda_matrix(const election_t & scores,
			size_t num_candidates_in, pairwise_type kind,
			bool cardinal, completion_type completion_in);
		cond_borda_matrix(const cond_borda_matrix & in,
			pairwise_type type_in);

		// set_num_cands - realloc when called??

		size_t get_num_candidates() const {
			return num_candidates;
		}
		void set_num_candidates(size_t candidates);

		// Add Range (cardinal distances) here later?
		void count_ballots(const election_t & scores,
			size_t num_candidates_in, bool cardinal,
			completion_type completion);

		void zeroize();

		bool reinit(completion_type completion);
		bool update();
};