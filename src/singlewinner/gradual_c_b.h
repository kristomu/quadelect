#ifndef _VOTE_GRAD_COPELAND
#define _VOTE_GRAD_COPELAND

#include "../pairwise/grad_matrix.h"
#include "../grad_median/grad_median.h"
#include "pairwise/method.h"
#include "method.h"

#include <iterator>
#include <iostream>

#include <glpk.h>
#include <assert.h>

using namespace std;

class gradual_cond_borda : public election_method {
	private:
		const pairwise_method * base_method;
		string cached_name;
		bool is_sym_comp, is_relaxed;
		completion_type completion;
		bool cardinal;

		string determine_name() const;

	public:
		pair<ordering, bool> elect_inner(
				const list<ballot_group> & papers,
				const vector<bool> & hopefuls,
				int num_candidates, cache_map & cache,
				bool winner_only) const;

		string name() const { return (cached_name); }

		gradual_cond_borda(pairwise_method * base_method_in,
				bool cardinal_in,
				completion_type completion_in);

};

#endif
