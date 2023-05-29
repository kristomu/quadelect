#ifndef _VOTE_3C_EXP
#define _VOTE_3C_EXP

#include "../../pairwise/matrix.h"
#include "../pairwise/method.h"
#include "../method.h"

#include <iterator>
#include <iostream>

#include <assert.h>

using namespace std;

enum texp_type {		TEXP_BPW = 0, 		TEXP_SV_VAR_1 = 1,
	TEXP_SV_VAR_2 = 2,	TEXP_SV_VAR_3 = 3,	TEXP_SV_VAR_4 = 4,
	TEXP_SV_VAR_5 = 5,	TEXP_SV_VAR_6 = 6,	TEXP_SV_VAR_7 = 7,
	TEXP_SV_VAR_8 = 8,	TEXP_SV_VAR_9 = 9,	TEXP_SV_VAR_10 = 10,
	TEXP_SV_VAR_11= 11,	TEXP_SV_VAR_12= 12,	TEXP_SV_VAR_13 = 13,
	TEXP_SV_VAR_14= 14, TEXP_SV_VAR_15= 15, TEXP_SV_VAR_16 = 16,
	TEXP_SV_VAR_17= 17, TEXP_SV_VAR_18= 18, TEXP_SV_VAR_19 = 19,
	TEXP_SV_VAR_20= 20, TEXP_SV_VAR_21= 21,	TEXP_SV_VAR_22 = 22,
	TEXP_SV_VAR_23= 23,	TEXP_SV_VAR_24= 24,	TEXP_SV_VAR_25 = 25,
	TEXP_SV_VAR_26= 26,	TEXP_SV_VAR_27= 27, TEXP_SV_VAR_28 = 28,
	TEXP_SV_VAR_29= 29, TEXP_SV_ORIG = 30
};

const int TEXP_TOTAL=31;

class three_experimental : public election_method {
	private:
		texp_type type;

	public:
		pair<ordering, bool> elect_inner(
			const list<ballot_group> & papers,
			const vector<bool> & hopefuls,
			int num_candidates, cache_map * cache,
			bool winner_only) const;

		string name() const {
			switch (type) {
				case TEXP_BPW: return ("EXP:Beat Plurality Winner");
				case TEXP_SV_VAR_1: return ("EXP:Three-Cand. Experiment (SV 1)");
				case TEXP_SV_VAR_2: return ("EXP:Three-Cand. Experiment (SV 2)");
				case TEXP_SV_VAR_3: return ("EXP:Three-Cand. Experiment (SV 3)");
				case TEXP_SV_VAR_4: return ("EXP:Three-Cand. Experiment (SV 4)");
				case TEXP_SV_VAR_5: return ("EXP:Three-Cand. Experiment (SV 5)");
				case TEXP_SV_VAR_6: return ("EXP:Three-Cand. Experiment (SV 6)");
				case TEXP_SV_VAR_7: return ("EXP:Three-Cand. Experiment (SV 7)");
				case TEXP_SV_VAR_8: return ("EXP:Three-Cand. Experiment (SV 8)");
				case TEXP_SV_VAR_9: return ("EXP:Three-Cand. Experiment (SV 9)");
				case TEXP_SV_VAR_10: return ("EXP:Three-Cand. Experiment (SV 10)");
				case TEXP_SV_VAR_11: return ("EXP:Three-Cand. Experiment (SV 11)");
				case TEXP_SV_VAR_12: return ("EXP:Three-Cand. Experiment (SV 12B)");
				case TEXP_SV_VAR_13: return ("EXP:Three-Cand. Experiment (SV 13)");
				case TEXP_SV_VAR_14: return ("EXP:Three-Cand. Experiment (SV 14)");
				case TEXP_SV_VAR_15: return ("EXP:Three-Cand. Experiment (SV 15)");
				case TEXP_SV_VAR_16: return ("EXP:Three-Cand. Experiment (SV 16)");
				case TEXP_SV_VAR_17: return ("EXP:Three-Cand. Experiment (SV 17)");
				case TEXP_SV_VAR_18: return ("EXP:Three-Cand. Experiment (SV 18)");
				case TEXP_SV_VAR_19: return ("EXP:Three-Cand. Experiment (SV 19)");
				case TEXP_SV_VAR_20: return ("EXP:Three-Cand. Experiment (SV 20)");
				case TEXP_SV_VAR_21: return ("EXP:Three-Cand. Experiment (SV 21)");
				case TEXP_SV_VAR_22: return ("EXP:Three-Cand. Experiment (SV 22)");
				case TEXP_SV_VAR_23: return ("EXP:Three-Cand. Experiment (SV 23)");
				case TEXP_SV_VAR_24: return ("EXP:Three-Cand. Experiment (SV 24)");
				case TEXP_SV_VAR_25: return ("EXP:Three-Cand. Experiment (Cond-Plur)");
				case TEXP_SV_VAR_26: return ("EXP:Three-Cand. Experiment (Brute 26)");
				case TEXP_SV_VAR_27: return ("EXP:Three-Cand. Experiment (Brute 27)");
				case TEXP_SV_VAR_28: return ("EXP:Three-Cand. Experiment (Brute 28C)");
				case TEXP_SV_VAR_29: return ("EXP:Three-Cand. Experiment (Brute 29)");
				case TEXP_SV_ORIG: return ("EXP:Three-Cand. Experiment (SV orig)");
				default: return ("EXP:Three-Cand. Experiment (UNKNOWN)");
			}
		}

		three_experimental(texp_type type_in) {
			type = type_in;
		}
};

#endif
