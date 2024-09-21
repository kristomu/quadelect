#include "generator/spatial/gaussian.h"
#include "random/random.h"
#include <vector>

#include "measure.h"

// Proportionality measure for normal spatial models. This tries to fit a
// normal distribution given the winners' coordinates as sampled points.

// It then does Monte-Carlo integration to check the Sainte-Laguë measure
// between the fitted distribution and the actual distribution; a better
// fit indicates that the returned winners are representative of the
// distribution.

class normal_proportionality : public proportionality_measure {
	private:
		gaussian_generator reference;
		std::vector<std::vector<double> > candidate_positions;
		rng rnd;

	public:
		// TODO, use a different seed
		normal_proportionality(gaussian_generator ref_in) : rnd(1) {
			reference = ref_in;
		}

		void prepare(const positions_election & p_e);
		double get_error(const std::list<size_t> & outcome);
};