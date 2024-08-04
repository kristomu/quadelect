#pragma once

#include "../bernoulli.h"

#include "common/ballots.h"

#include "generator/ballotgen.h"
#include "singlewinner/method.h"
#include "random/random.h"

#include "generator/spatial/gaussian.h"

// This class calculates a linearized version of VSE/SUE (vote
// satisfaction efficiency, social utility efficiency), suitable for use in
// bandit searches.

// The VSE for method M under ballot model G is defined as follows:

//               E[chosen(M,G)] - E[random(G)]
// VSE(M,G)  =   -----------------------------
//               E[optimal(G)] - E[random(G)]

// Where E[chosen] is the expected social utility of candidates chosen by
// method M when given random elections from model G, E[optimal(G)] is
// the expected utility of the highest utility candidates across random
// elections drawn from G, and E[random(G)] is the expected utility of
// random candidates in random elections drawn from G.

// Suppose we let the sample VSE after n iterations to be equal to the
// sample means for the different expectations. To apply multi-armed bandit
// we need linearity, in the sense that VSE can be considered a sample mean
// of another distribution. This is clearly not the case for exact VSE above,
// but we can make use of

// E[optimal(G)] - E[random(G)]

// being a constant that's independent of M. If we extract out this constant c,
// we get a population mean:

// VSE^(M,G)_n = chosen(M,G)^/n - random(G)^/n * 1/c
//             = (chosen(M,G)^ - random(G)^)/n

// and we can use multi-armed bandit due to the linearity of expectation.
// (The hats here denote sample totals). This converges to the true VSE in the
// limit of n going to infinity.

// Ideally we'd exactly calculate c, but this is very hard. But this is not a
// problem for multi-armed bandit, because the 1/c factor doesn't affect the
// relative performance of different methods M, so the multi-armed bandit search
// is unaffected by an inaccurate c as long as we use the same c for every M. The
// only thing we lose is result accuracy.

// NOTE: It's not a very good idea to use c to scale the accumulated results,
// since we can't calculate it exactly. So either I should do a refactor (remove
// c entirely and clean up the explanation above), or find a way to calculate it
// asymptotically.

// TODO: The above should also mention that E[random(G)] is also present in the
// numerator, and that the same reasoning applies here, too. Thus we can remove
// rangom(G)^/n entirely from the bandit's consideration. This will reduce the
// variance and variance proxy and possibly lead to better bandit convergence.

// TODO: Figure out why I get so good VSE results with so many methods being
// close to 0.99. Compare to JGA's results and find a method/generator pair
// that he's provided results for -- then start debugging.

// Now how do I determine the variance proxy based on the ballot generator?
// Yuck! Force a Gaussian for now and let the variance proxy be the variance
// of a chi distribution - justification later.

class vse_sim : public simulator {
	private:
		size_t numcands, numvoters;

		std::shared_ptr<election_method> method;
		gaussian_generator ballot_gen;
		double E_opt_rand;
		double sigma;

		// These are the exact total optimum and random
		// candidate utilities for the draws done so far.
		// They are used for turning the linearized, approximately
		// correct outcome into the actual finite outcome, for
		// display and brute purposes.
		// NOTE: The random utility value is a sum of means per
		// simultion; they're not a total over both simulations
		// and candidates!
		double exact_random, exact_optimum;

		double do_simulation_inner();

	protected:
		double do_simulation();

	public:
		bool higher_is_better() const {
			return true;
		}

		vse_sim(std::shared_ptr<coordinate_gen> entropy_src_in,
			std::shared_ptr<election_method> method_in,
			size_t numcands_in, size_t numvoters_in,
			size_t dimensions_in) : simulator(entropy_src_in) {

			reset();

			numcands = numcands_in;
			numvoters = numvoters_in;
			method = method_in;
			E_opt_rand = -1;

			set_dispersion(ballot_gen.get_dispersion()[0]);
			set_dimensions(dimensions_in);
		}

		void reset() {
			exact_random = 0;
			exact_optimum = 0;
			simulator::reset();
		}

		double get_exact_value(
			double linearized, bool total) const;

		// For investigating the convergence between a linearly scaled
		// linearized value and the exact value.
		double get_adjusted_linear_value() const {
			return get_linearized_mean_score() * E_opt_rand;
		}

		// Also return generator name?
		std::string name() const {
			return "VSE[" + method->name() + "]";
		}

		void set_scale_factor(double E_opt_rand_in) {
			E_opt_rand = E_opt_rand_in;
		}

		bool set_dimensions(size_t num_dimensions_in) {
			return ballot_gen.set_params(num_dimensions_in, false);
		}

		bool set_dispersion(double dispersion_in) {
			sigma = dispersion_in;
			return ballot_gen.set_dispersion(dispersion_in);
		}

		double variance_proxy() const;
};
