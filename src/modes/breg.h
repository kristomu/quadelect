// Possible BLUESKY: Support different utility aggregation functions. Mean,
// median, interquartile range, minimum, maximum, mean with Gini penalty, etc...

// Hm, have some kind of logic where if you add a method, it resets? Some way
// of signaling this?

#ifndef _VOTE_MODE_BR
#define _VOTE_MODE_BR

#include "mode.h"
#include "../stats/stats.h"
#include "../generator/ballotgen.h"
#include "../singlewinner/method.h"
#include "../singlewinner/stats/cardinal.h"

#include <vector>
#include <list>

#include <assert.h>
#include <values.h>

using namespace std;

class bayesian_regret : public mode {

	private:
		bool inited;
		int maxiters, curiter;
		int min_candidates, max_candidates;
		int min_voters, max_voters;
		bool show_median;

		stats_type br_type;

		vector<pure_ballot_generator *> generators;
		vector<const election_method *> methods;
		vector<stats<float> > method_stats;

		vector<double> utilities;

	public:

		bayesian_regret();

		bool set_maxiters(int maxiters_in, bool clear_curiters);
		bool set_num_candidates(int min, int max);
		bool set_num_voters(int min, int max);
		void set_format(bool do_show_median);
		// Altering the statistical type will clear the stats!
		void set_br_type(const stats_type br_type_in);

		void add_generator(pure_ballot_generator * to_add);
		void add_method(const election_method * to_add);
		template<typename T> void add_generators(T start_iter, 
				T end_iter);
		template<typename T> void add_methods(T start_iter,
				T end_iter);

		void clear_generators(bool do_delete);
		void clear_methods(bool do_delete);
		void reset_round_count() { curiter = 0; }

		bool set_parameters(int maxiters_in, int curiter_in,
				int min_cand_in, int max_cand_in,
				int min_voters_in, int max_voters_in,
				bool show_median_in, stats_type br_type_in,
				list<pure_ballot_generator *> & generators_in,
				list<const election_method *> & methods_in);

		bayesian_regret(int maxiters_in, 
				int min_cand_in, int max_cand_in,
				int min_voters, int max_voters,
				bool show_median_in, stats_type br_type_in,
				list<pure_ballot_generator *> & generators_in,
				list<const election_method *> & methods_in);

		// Create stats for a single method given by its index
		// in the methods array.
		bool init_one(size_t idx);
		bool init(rng & randomizer); // This will also clear stats.

		int get_max_rounds() const { return(maxiters); }

		// -1 if nothing's going on?
		int get_current_round() const { return(curiter); }

		// But what of cache sharing with other instances?
		// TODO, fix that so we can have one INTRAROUND and one
		// INTERROUND going on at the same time, for instance.
		// Hm, that will be harder than I thought... I may need
		// hash names to do that properly.
		string do_round(bool give_brief_status, bool reseed,
				rng & randomizer, cache_map * cache);

		string do_round(bool give_brief_status, bool reseed,
				rng & randomizer);

		vector<string> provide_status() const; 
};

// These have to be in the header since they're templated functions.
template<typename T> void bayesian_regret::add_generators(T start_iter,
		T end_iter) {

	for (T iterator = start_iter; iterator != end_iter; ++iterator)
		add_generator(*iterator);
}

template<typename T> void bayesian_regret::add_methods(T start_iter,
		T end_iter) {

	for (T iterator = start_iter; iterator != end_iter; ++iterator)
		add_method(*iterator);
}

#endif
