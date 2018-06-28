// TODO: Handle incomplete ballots by considering everybody not ranked to be
// ranked equal last.

#ifndef _VOTE_EXT_ANTIPLUR
#define _VOTE_EXT_ANTIPLUR

#include "ext_plurality.h"

class ext_antiplurality : public positional {
	private:
		ext_plurality inner_method;

	protected:
		virtual double pos_weight(int position, int last_position) const {
			// This should never be called.
			throw std::logic_error(
				"ext_antiplurality pos_weight should not be called!");
		}

	public:
		ordering pos_elect(const vector<vector<double> > &
				positional_matrix, int num_hopefuls,
				const vector<bool> * hopefuls) const {

			// We can get Antiplurality from Plurality by reversing all the
			// ballots, running Plurality, and reversing the result. In this
			// case, we reverse the positional matrix instead.

			int numcands = (int)positional_matrix.size();

			std::vector<vector<double > > rev_pos_matrix;

			for (size_t i = 0; i < positional_matrix.size(); ++i) {
				rev_pos_matrix.push_back(vector<double>(
					positional_matrix[i].rbegin(),
					positional_matrix[i].rend()));
			}

			ordering rev_plurality_ordering = inner_method.pos_elect(
				rev_pos_matrix, num_hopefuls, hopefuls), social_order;

			for (ordering::const_iterator pos = rev_plurality_ordering.begin();
					pos != rev_plurality_ordering.end(); ++pos) {

				int curcand = pos->get_candidate_num();
				if (hopefuls != NULL && !(*hopefuls)[curcand]) {
					social_order.insert(candscore(curcand, -numcands-1));
				} else {
					social_order.insert(candscore(curcand, 
						-pos->get_score()));
				}
			}

			return(social_order);
		}

		ext_antiplurality(positional_type 
			kind_in) : positional(kind_in), inner_method(kind_in) {}

		string pos_name() const { return("Ext-Antiplurality"); }
};

#endif