// Base class for positional voting methods.

#ifndef _VOTE_POSITIONAL
#define _VOTE_POSITIONAL

#include "common/ballots.h"
#include "../method.h"
#include <list>
#include <vector>

#include "types.h"


class positional : public election_method {

	private:
		// Used for quota methods.
		double get_weight_sum(const election_t & input) const;

		ordering elect_to_ordering(const election_t & input,
			size_t num_candidates, size_t num_hopefuls,
			const std::vector<bool> & hopefuls) const;

		std::string show_type(const positional_type & kind_in) const;

	protected:
		positional_type kind;

		virtual std::string pos_name() const = 0;

		// For things like Plurality, this returns the position at
		// which all further weights will be zero.
		virtual size_t zero_run_beginning() const {
			return (SIZE_MAX);
		}
		// 0..last_position inclusive
		virtual double pos_weight(size_t position,
			size_t last_position) const = 0;

	public:
		// Perhaps this should be in positional_aggregator.
		// It should be. TODO.
		double get_pos_score(const ballot_group & input,
			size_t candidate_number,
			const std::vector<bool> & hopefuls,
			size_t num_hopefuls) const;

		double get_pos_maximum(size_t num_candidates) const {
			return (pos_weight(0, num_candidates-1));
		}

		// Interfaces
		// Beware: still uses int for numcands.

		std::pair<ordering, bool> elect_inner(
			const election_t & input,
			const std::vector<bool> & hopefuls,
			int num_candidates, cache_map * cache,
			bool winner_only) const;

		// Particular interface to all positional methods.
		virtual ordering pos_elect(
			const std::vector<std::vector<double> > &
			positional_matrix, int num_hopefuls,
			const std::vector<bool> & hopefuls) const;

		positional(positional_type kind_in) {
			kind = kind_in;
		}

		std::string show_type() const {
			return show_type(kind);
		}

		std::string name() const;

};

#endif
