#pragma once

#include "../bernoulli.h"

#include "../../ballots.h"

#include "../../generator/ballotgen.h"
#include "../../singlewinner/method.h"
#include "../../random/random.h"

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

// TODO: MAB also needs bounded results. Fix later.
// TODO: Implement scale factor c.

class vse_sim : public simulator {
	private:
		std::shared_ptr<election_method> method;
		std::shared_ptr<pure_ballot_generator> ballot_gen;

	protected:
		double do_simulation();

	public:
		bool higher_is_better() const {
			return true;
		}

		vse_sim(std::shared_ptr<coordinate_gen> entropy_src_in,
			std::shared_ptr<election_method> method_in,
			std::shared_ptr<pure_ballot_generator>
			generator_in) : simulator(entropy_src_in) {

			method = method_in;
			ballot_gen = generator_in;
		}

		// Also return generator name?
		std::string name() const {
			return "VSE[" + method->name() + "]";
		}

		// These must also be populated somehow. While maximum
		// would be no problem sans our linearization trick, it may
		// become a problem with it. And minimum can still be
		// theoretically unbounded. We need to poll the proper
		// information from the distribution and then clamp on
		// anything more extreme than 1 ppm quantile.
		double get_minimum() const {
			return -1;    // ????
		}
		double get_maximum() const {
			return 1;    // ???
		}
};
