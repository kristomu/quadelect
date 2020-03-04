#ifndef _VOTE_EXT_PLUR
#define _VOTE_EXT_PLUR

#include "../../ballots.h"
#include "../method.h"
#include <list>
#include <vector>
#include <stdexcept>

#include "types.h"

#include "positional.h"

class ext_plurality : public positional {

	protected:
		virtual double pos_weight(size_t position, size_t last_position) const {
			// This should never be called.
			throw std::logic_error(
				"ext_plurality pos_weight should not be called!");
		}

	public:
		ordering pos_elect(const vector<vector<double> > &
				positional_matrix, int num_hopefuls,
				const vector<bool> * hopefuls) const {

			// We want to find the permutation x_1..x_n so that
			// the x_ith row of the positional matrix is always >=
			// the x_jth row if i<j, where the comparison operator
			// is leximax. Thus we'll break ties in first place based
			// on second place, ties in first and second based on third,
			// and so on.

			std::vector<std::pair<std::vector<double>, int> >
				scores_per_candidate;

			for (size_t i = 0; i < positional_matrix.size(); ++i) {
				scores_per_candidate.push_back(
					std::pair<std::vector<double>, int>(positional_matrix[i],
					i));
			}

			// Sort ascending
			std::sort(scores_per_candidate.begin(),
				scores_per_candidate.end());
			// Reverse to get descending
			std::reverse(scores_per_candidate.begin(),
				scores_per_candidate.end());

			// Now just read off the sorted entries and check if they're
			// equal.

			int rank_counter = 0;
			ordering social_order;

			for (size_t i = 0; i < positional_matrix.size(); ++i) {
				if (i != 0 && scores_per_candidate[i].first !=
					scores_per_candidate[i-1].first) {
					--rank_counter;
				}

				if (hopefuls == NULL || (*hopefuls)[scores_per_candidate[i].second]) {
					social_order.insert(candscore(scores_per_candidate[i].second,
						rank_counter));
				}
			}

			return(social_order);
		}

		ext_plurality(positional_type kind_in) : positional(kind_in) {}

		string pos_name() const { return("Ext-Plurality"); }
};

#endif