#pragma once

#include "simple_methods.h"
#include "positional.h"

class bucklin : public positional {
	private:
		mutable sweep sweep_method;

	protected:
		virtual double pos_weight(size_t position,
			size_t last_position) const {

			// This should never be called.
			throw std::logic_error(
				"Bucklin: pos_weight should not be called!");
		}

	public:
		ordering pos_elect(const std::vector<std::vector<double> > &
			positional_matrix, int num_hopefuls,
			const std::vector<bool> * hopefuls) const {

			size_t i;

			// Get the number of voters so we know what a majority is.
			size_t numcands = positional_matrix.size();
			double numvoters = 0;

			for (size_t i = 0; i < positional_matrix.size(); ++i) {
				numvoters += positional_matrix[i][0];
			}

			// Repeatedly run the sweep until the highest ranked candidate
			// has more than a majority.
			for (i = 0; i < numcands; ++i) {
				sweep_method.set_sweep_point(i+1);
				ordering social_order = sweep_method.pos_elect(
						positional_matrix, num_hopefuls, hopefuls);

				if (social_order.begin()->get_score() >= numvoters * 0.5) {
					// Always set the sweep to 1 after we're done, as an
					// invariant.
					sweep_method.set_sweep_point(1);

					return social_order;
				}
			}

			throw std::logic_error(
				"Bucklin: Didn't get a majority!");
		}

		bucklin() : positional(PT_WHOLE), sweep_method(PT_WHOLE) {}

		std::string pos_name() const {
			return ("Bucklin");
		}
};