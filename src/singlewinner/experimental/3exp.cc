#include "3exp.h"
#include "../positional/simple_methods.h"
#include "../pairwise/simple_methods.h"

std::pair<ordering, bool> three_experimental::elect_inner(
	const election_t & papers,
	const std::vector<bool> & hopefuls,
	int num_candidates, cache_map * cache,
	bool winner_only) const {

	// TODO: Use cache.

	condmat condorcet_matrix = condmat(papers, num_candidates,
			CM_PAIRWISE_OPP);

	assert(condorcet_matrix.get_num_candidates() <= 3);

	int num_hopefuls = 0;
	for (bool hopeful : hopefuls) {
		if (hopeful) {
			++num_hopefuls;
		}
	}

	// If two candidates or less, just hand off to a method that does a majority vote.
	if (num_hopefuls < 3) {
		return ord_minmax(CM_PAIRWISE_OPP).pair_elect(
				condorcet_matrix, hopefuls, cache, winner_only);
	}

	// Get Plurality scores. Whole or fractional? Should perhaps be set
	// as a parameter...
	// Hopefuls???

	ordering plur_ordering = plurality(PT_WHOLE).elect(papers,
			num_candidates, cache, false);

	// IDEA: multiply the score term by plur_scores[counter]*plur_scores[sec]

	std::vector<double> plur_scores(num_candidates, 0);
	double numvoters = 0;

	for (ordering::const_iterator pos = plur_ordering.begin(); pos !=
		plur_ordering.end(); ++pos) {
		plur_scores[pos->get_candidate_num()] = pos->get_score();
		numvoters += pos->get_score();
	}

	std::vector<double> plur_factors(num_candidates, 0);
	int counter;
	for (counter = 0; counter < num_candidates; ++counter) {
		plur_factors[counter] = 0.5 - plur_scores[counter]/numvoters;
	}

	ordering out;

	for (int counter = 0; counter < 3; ++counter) {
		double score = 0;

		// Cycle A>B>C>A
		// then counter = 0 (A)
		// sec = 1 (B),
		// third party = 2 (C)

		// SV would presumably be:
		// (B>C) / (C>A)

		//double majority = 0.5 * input.get_num_voters();

		for (int sec = 0; sec < 3; ++sec) {
			if (counter == sec) {
				continue;
			}

			int third_party = -1;
			for (int tet = 0; tet < 3 && third_party == -1; ++tet)
				if (counter != tet && sec != tet) {
					third_party = tet;
				}

			if (condorcet_matrix.get_magnitude(counter, sec) >=
				condorcet_matrix.get_magnitude(sec, counter)) {
				double eps = 1e-5;

				switch (type) {
					case TEXP_BPW:
						score += plur_scores[sec];
						break;
					case TEXP_SV_VAR_1:
						// - f_beating/f_third party
						score -= plur_factors[sec]/plur_factors[third_party];
						break;
					case TEXP_SV_VAR_2:      // TODO: REMOVE
						// f_third_party/f_beating
						score += plur_factors[third_party]/plur_factors[sec];
						break;
					case TEXP_SV_VAR_3:
						// multiplications with epsilon, 1
						score += (eps+plur_scores[sec])*(eps+plur_scores[sec])*
							(eps+plur_scores[counter]);
						break;
					case TEXP_SV_VAR_4:      // TODO: REMOVE
						// multiplications with epsilon, 2
						score += (eps+plur_scores[sec]*plur_scores[sec])*(eps
								+plur_scores[counter]);
						break;
					case TEXP_SV_VAR_5:
						// with other powers
						// There are some x,y that minimize the strategy rate, but I have no idea
						// what they are.
						score += pow(eps+plur_scores[sec], 2.3) * pow(eps+plur_scores[counter],
								0.9);
						break;
					case TEXP_SV_VAR_6:
						score -= (eps+plur_scores[third_party]*plur_scores[third_party])*
							(eps+plur_scores[sec]);
						break;
					case TEXP_SV_VAR_7:
						score += std::min(plur_scores[sec],
								plur_scores[counter]) + eps * std::max(plur_scores[sec],
								plur_scores[counter]);
						break;
					case TEXP_SV_VAR_8:
						score += std::min(plur_scores[sec],
								plur_scores[third_party]) + eps * std::max(plur_scores[sec],
								plur_scores[third_party]);
						break;
					case TEXP_SV_VAR_9: // REMOVE
						score += std::min(plur_scores[counter],
								plur_scores[third_party]) + eps * std::max(plur_scores[counter],
								plur_scores[third_party]);
						break;
					case TEXP_SV_VAR_10:
						score += std::max(plur_scores[sec],
								plur_scores[counter]) + eps * std::min(plur_scores[sec],
								plur_scores[counter]);
						break;
					case TEXP_SV_VAR_11:
						score += std::max(plur_scores[sec],
								plur_scores[third_party]) + eps * std::min(plur_scores[sec],
								plur_scores[third_party]);
						break;
					case TEXP_SV_VAR_12: // REMOVE
						//score += std::max(plur_scores[counter], plur_scores[third_party]) + eps * std::min(plur_scores[counter], plur_scores[third_party]);
						score += plur_scores[counter] - plur_scores[third_party];
						break;
					case TEXP_SV_VAR_13: // REMOVE
						score += std::min(plur_factors[sec],
								plur_factors[counter]) + eps * std::max(plur_factors[sec],
								plur_factors[counter]);
						break;
					case TEXP_SV_VAR_14:
						score += std::min(plur_factors[sec],
								plur_factors[third_party]) + eps * std::max(plur_factors[sec],
								plur_factors[third_party]);
						break;
					case TEXP_SV_VAR_15:
						score += std::min(plur_factors[counter],
								plur_factors[third_party]) + eps * std::max(plur_factors[counter],
								plur_factors[third_party]);
						break;
					case TEXP_SV_VAR_16: // REMOVE
						score += std::max(plur_factors[sec],
								plur_factors[counter]) + eps * std::min(plur_factors[sec],
								plur_factors[counter]);
						break;
					case TEXP_SV_VAR_17:
						score += std::max(plur_factors[sec],
								plur_factors[third_party]) + eps * std::min(plur_factors[sec],
								plur_factors[third_party]);
						break;
					case TEXP_SV_VAR_18:
						score += std::max(plur_factors[counter],
								plur_factors[third_party]) + eps * std::min(plur_factors[counter],
								plur_factors[third_party]);
						break;
					case TEXP_SV_VAR_19:
						// A>B = ABC + ACB + CAB
						score += plur_scores[counter] + plur_scores[sec] +
							eps*condorcet_matrix.get_magnitude(counter, sec);
						break;
					case TEXP_SV_VAR_20:
						score += pow(plur_scores[counter], 2.1) + pow(plur_scores[sec], 1.8);
						break;
					case TEXP_SV_VAR_21:
						score += plur_scores[counter] * plur_scores[sec] +
							eps*condorcet_matrix.get_magnitude(counter, sec);
						break;
					case TEXP_SV_VAR_22:
						score += plur_scores[counter] + plur_scores[sec] - plur_scores[third_party]
							+ eps*condorcet_matrix.get_magnitude(counter, sec);
						break;
					case TEXP_SV_VAR_23:
						score += 2 * plur_scores[counter] + plur_scores[sec] +
							eps*condorcet_matrix.get_magnitude(counter, sec);
						break;
					case TEXP_SV_VAR_24:
						score += plur_scores[counter] + plur_scores[sec] +
							eps*condorcet_matrix.get_magnitude(counter, sec);
						break;
					case TEXP_SV_VAR_25:
						score += plur_scores[counter];
						break;
					case TEXP_SV_VAR_26: // monotone
						score += plur_scores[sec] + condorcet_matrix.get_magnitude(counter,
								sec) - 1.1 * plur_scores[third_party];
						break;
					case TEXP_SV_VAR_27: // NOT. Needs to use pairwise opposition.
						score += condorcet_matrix.get_magnitude(counter,
								third_party) - plur_scores[counter] +
							condorcet_matrix.get_magnitude(third_party,
								counter) - plur_scores[third_party];
						break;
					case TEXP_SV_VAR_28:
						// Another possibility
						//score += (1+eps) * condorcet_matrix.get_magnitude(counter, sec) + plur_scores[counter] + 2 * plur_scores[sec];
						// score += 2 * (plur_scores[sec] + condorcet_matrix.get_magnitude(counter, sec)) - plur_scores[third_party];
						// A>B * std::min(C>A, A>B)/fpC
						score += condorcet_matrix.get_magnitude(counter,
								sec) * std::min(condorcet_matrix.get_magnitude(counter, sec),
								condorcet_matrix.get_magnitude(third_party,
									counter))/(plur_scores[third_party]+eps);
						break;
					case TEXP_SV_VAR_29: {
						// ABC/ACB - CBA/BCA

						// ABC: ABC
						// Reversal symmetric but not monotone
						double ABC = condorcet_matrix.get_magnitude(sec,
								third_party) - plur_scores[sec];
						//double ACB = condorcet_matrix.get_magnitude(third_party, sec) - plur_scores[third_party];
						double CBA = condorcet_matrix.get_magnitude(sec,
								counter) - plur_scores[sec];
						double BCA = condorcet_matrix.get_magnitude(third_party,
								counter) - plur_scores[third_party];

						//score -= ABC/(ACB+eps) + CBA/(BCA+eps);
						// CBA BCA ABC MIN CBA MAX /
						score += std::max(CBA, std::min(BCA, ABC)) / (CBA+eps);

						break;
					}
					case TEXP_SV_ORIG:
						score += plur_scores[sec]/(eps+plur_scores[third_party]);
						break;

				}

				//if (type == TEXP_BPW)
				//        score += plur_scores[sec];
				//else
				// There are some x,y that minimize the strategy rate, but I have no idea
				// what they are.
				//score += pow(0.001+plur_scores[sec], 2.3) * pow(0.001+plur_scores[counter], 0.9);

				//score += (double)(plur_scores[sec])/(1e-9 + plur_scores[third_party]);
				//score += (double)(eps+plur_scores[sec])*(eps+plur_scores[sec])*(eps+plur_scores[counter]);
			}
		}

		out.insert(candscore(counter, score));
	}

	return (std::pair<ordering, bool>(out, false));
}
