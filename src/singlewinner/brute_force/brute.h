// The linear brute-forcing method for three candidates.
// If there's a CW, he wins. If there's a tie between two candidates, both win.
// If there are more than three candidates, not supported.
// Otherwise, there's a cycle and without loss of generality we can suppose that
// cycle is ABCA (by relabeling the candidates). Then A's score is
// (ABC, ACB, ..., CBA) dot (w[0], w..., w[5]). To get B, we just relabel the
// candidates again. Greatest score wins.

#pragma once

#include "../../pairwise/matrix.h"
#include "../pairwise/method.h"
#include "../method.h"

#include <iterator>
#include <iostream>
#include <assert.h>


class cond_brute : public election_method {
	private:
		std::vector<int> weights;
		int weight_code;
		int radix;

		std::vector<int> decode_weight_code(int wcode, int base) const;

	public:
		bool failed_liia;

		std::pair<ordering, bool> elect_inner(
			const election_t & papers,
			const std::vector<bool> & hopefuls,
			int num_candidates, cache_map * cache,
			bool winner_only) const;

		std::string name() const {
			std::string compliances = get_compliances();
			if (compliances != "") {
				compliances = "/" + compliances;
			}

			return ("Brute linear("+itos(weight_code)+", "+itos(radix)+")" +
					compliances);
		}

		cond_brute(int weight_code_in, int radix_in) {
			radix = radix_in;
			weight_code = weight_code_in;
			weights = decode_weight_code(weight_code, radix);
			failed_liia = false;
		}

		cond_brute(int weight_code_in) : cond_brute(weight_code_in, 5) {}

		// These should also be refactored to overload the appropriate checkers.
		// TODO: Implement also "is "
		bool is_monotone() const;
		bool passes_mat() const;
		bool reversal_symmetric() const;
		std::string get_compliances() const;
};