#ifndef _VOTE_3C_EXP
#define _VOTE_3C_EXP

#include "../../pairwise/matrix.h"
#include "../pairwise/method.h"
#include "../method.h"

#include <iterator>
#include <iostream>

#include <assert.h>

using namespace std;

enum texp_type { TEXP_BPW, TEXP_SV_VAR_1, TEXP_SV_VAR_2, TEXP_SV_VAR_3, 
	TEXP_SV_VAR_4, TEXP_SV_VAR_5, TEXP_SV_VAR_6, TEXP_SV_VAR_7,
	TEXP_SV_VAR_8, TEXP_SV_VAR_9, TEXP_SV_VAR_10, TEXP_SV_VAR_11, TEXP_SV_VAR_12,
	TEXP_SV_VAR_13, TEXP_SV_VAR_14, TEXP_SV_VAR_15, TEXP_SV_VAR_16,
	TEXP_SV_VAR_17, TEXP_SV_VAR_18, TEXP_SV_VAR_19, TEXP_SV_VAR_20, TEXP_SV_VAR_21,
	TEXP_SV_VAR_22, TEXP_SV_VAR_23, TEXP_SV_VAR_24, TEXP_SV_VAR_25, TEXP_SV_VAR_26,
	TEXP_SV_VAR_27, TEXP_SV_VAR_28, TEXP_SV_VAR_29, TEXP_SV_ORIG };

class three_experimental : public election_method {
	private:
		texp_type type;

	public:
		pair<ordering, bool> elect_inner(
				const list<ballot_group> & papers,
				const vector<bool> & hopefuls,
				int num_candidates, cache_map & cache,
				bool winner_only) const;

		string name() const { 
			switch(type) {
				case TEXP_BPW: return("Beat Plurality Winner");
				case TEXP_SV_VAR_1: return("Three-Cand. Experiment (SV 1)");
				case TEXP_SV_VAR_2: return("Three-Cand. Experiment (SV 2)");
				case TEXP_SV_VAR_3: return("Three-Cand. Experiment (SV 3)");
				case TEXP_SV_VAR_4: return("Three-Cand. Experiment (SV 4)");
				case TEXP_SV_VAR_5: return("Three-Cand. Experiment (SV 5)");
				case TEXP_SV_VAR_6: return("Three-Cand. Experiment (SV 6)");
				case TEXP_SV_VAR_7: return("Three-Cand. Experiment (SV 7)");
				case TEXP_SV_VAR_8: return("Three-Cand. Experiment (SV 8)");
				case TEXP_SV_VAR_9: return("Three-Cand. Experiment (SV 9)");
				case TEXP_SV_VAR_10: return("Three-Cand. Experiment (SV 10)");
				case TEXP_SV_VAR_11: return("Three-Cand. Experiment (SV 11)");
				case TEXP_SV_VAR_12: return("Three-Cand. Experiment (SV 12B)");
				case TEXP_SV_VAR_13: return("Three-Cand. Experiment (SV 13)");
				case TEXP_SV_VAR_14: return("Three-Cand. Experiment (SV 14)");
				case TEXP_SV_VAR_15: return("Three-Cand. Experiment (SV 15)");
				case TEXP_SV_VAR_16: return("Three-Cand. Experiment (SV 16)");
				case TEXP_SV_VAR_17: return("Three-Cand. Experiment (SV 17)");
				case TEXP_SV_VAR_18: return("Three-Cand. Experiment (SV 18)");
				case TEXP_SV_VAR_19: return("Three-Cand. Experiment (SV 19)");
				case TEXP_SV_VAR_20: return("Three-Cand. Experiment (SV 20)");
				case TEXP_SV_VAR_21: return("Three-Cand. Experiment (SV 21)");
				case TEXP_SV_VAR_22: return("Three-Cand. Experiment (SV 22)");
				case TEXP_SV_VAR_23: return("Three-Cand. Experiment (SV 23)");
				case TEXP_SV_VAR_24: return("Three-Cand. Experiment (SV 24)");
				case TEXP_SV_VAR_25: return("Three-Cand. Experiment (Cond-Plur)");
				case TEXP_SV_VAR_26: return("Three-Cand. Experiment (Brute 26)");
				case TEXP_SV_VAR_27: return("Three-Cand. Experiment (Brute 27)");
				case TEXP_SV_VAR_28: return("Three-Cand. Experiment (Brute 28C)");
				case TEXP_SV_VAR_29: return("Three-Cand. Experiment (Brute 29)");
				case TEXP_SV_ORIG: return("Three-Cand. Experiment (SV orig)");
				default: return("Three-Cand. Experiment (UNKNOWN)");
			}
		}

		three_experimental(texp_type type_in) { type = type_in; }
};

#endif
