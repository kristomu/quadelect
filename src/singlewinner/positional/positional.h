// Base class for positional voting methods.

#ifndef _VOTE_POSITIONAL
#define _VOTE_POSITIONAL

#include "../../ballots.h"
#include "../method.h"
#include <list>
#include <vector>

#include "types.h"

using namespace std;

class positional : public election_method {

	private:
		// Used for quota methods.
		double get_weight_sum(const list<ballot_group> & input) const;

		ordering elect_to_ordering(const list<ballot_group> & input,
			size_t num_candidates, size_t num_hopefuls,
			const vector<bool> * hopefuls) const;

		string show_type(const positional_type & kind_in) const;

		// Perhaps this should be in positional_aggregator.
		// It should be. Fix later.
		double get_pos_score(const ballot_group & input,
			size_t candidate_number,
			const vector<bool> * hopefuls,
			size_t num_hopefuls) const;
		double get_pos_score(const ballot_group & input,
			size_t candidate_number, size_t num_candidates) const;

		double get_pos_maximum(size_t num_candidates) const {
			assert(num_candidates > 0);
			return (pos_weight(0, num_candidates-1));
		}

	protected:
		positional_type kind;

		virtual string pos_name() const = 0;

		// For things like Plurality, this returns the position at
		// which all further weights will be zero.
		virtual size_t zero_run_beginning() const {
			return (SIZE_MAX);
		}
		// 0..last_position inclusive
		virtual double pos_weight(size_t position,
			size_t last_position) const = 0;

	public:
		// Interfaces
		// Beware: still uses int for numcands.
		pair<ordering, bool> elect_inner(
			const list<ballot_group> & input,
			int num_candidates, cache_map * cache,
			bool winner_only) const;

		pair<ordering, bool> elect_inner(
			const list<ballot_group> & input,
			const vector<bool> & hopefuls,
			int num_candidates, cache_map * cache,
			bool winner_only) const;

		// Particular interface to all positional methods.
		virtual ordering pos_elect(const vector<vector<double> > &
			positional_matrix, int num_hopefuls,
			//double weight_sum,
			const vector<bool> * hopefuls) const;

		positional(positional_type kind_in) {
			kind = kind_in;
		}

		string name() const;

};

#endif
