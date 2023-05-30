// Positional vote matrix. This is a matrix that indexes how many voters voted
// each candidate first, second, third, and so on.

// (Perhaps cache this later.)

#ifndef _VOTE_POS_MATRIX
#define _VOTE_POS_MATRIX

#include "../../ballots.h"
#include "../method.h"
#include <list>
#include <vector>

#include "types.h"


// PT_WHOLE gives equal score to all that are equally ranked. PT_FRACTIONAL
// shares the point among all of them.

// TODO: some way of caching num_hopefuls. Also, "zero after this point" helper
// function (returns -1 by default) so that Plurality (for instance) doesn't
// have to go through all the candidates, just the first non-eliminated one.

// TODO also: show PT_WHOLE vs PT_FRACTIONAL in name. That'll require
// the determine_name / cache lookup here as well, but only in the abstract
// class. Actually, we need to do so, as otherwise there will be cache
// collisions whenever we use both.

// This turns a ballot group into a std::vector<double> of size equal to
// num_candidates. The vector is used both in ordinary positional methods,
// and in sweep-ones like Bucklin, where it reduces the naive n^3 complexity
// to n^2.
class positional_aggregator {

	private:
		// Used for moving back and forth on equal rank.
		template<typename T> T first(T cur, T end, const std::vector<bool> *
			hopefuls) const;
		template<typename T> T next(T cur, T end, const std::vector<bool> *
			hopefuls) const;

	public:
		void aggregate(const ballot_group & input,
			int num_candidates, int num_hopefuls,
			const std::vector<bool> * hopefuls,
			std::vector<std::vector<double> > & positional_array,
			positional_type kind,
			int zero_run_advice) const;
		// Perhaps protected, with only the one without zero_run_advice
		// available to the public.
		std::vector<std::vector<double> > get_positional_matrix(const
			std::list<ballot_group> & ballots,
			int num_candidates, int num_hopefuls,
			const std::vector<bool> * hopefuls,
			positional_type kind,
			int zero_run_advice) const;

		std::vector<std::vector<double> > get_positional_matrix(const
			std::list<ballot_group> & ballots,
			int num_candidates, int num_hopefuls,
			const std::vector<bool> * hopefuls,
			positional_type kind) const;
};

#endif
