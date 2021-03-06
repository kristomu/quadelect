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

#include <values.h>

class bayesian_regret : public mode {

	private:
		bool inited;
		size_t maxiters, curiter;
		size_t min_candidates, max_candidates;
		size_t min_voters, max_voters;
		bool show_median;

		stats_type br_type;

		vector<pure_ballot_generator *> generators;
		vector<const election_method *> methods;
		vector<stats<float> > method_stats;

		vector<double> utilities;

	public:

		bayesian_regret();

		void set_maxiters(size_t maxiters_in, bool clear_curiters);
		void set_num_candidates(size_t min, size_t max);
		void set_num_voters(size_t min, size_t max);
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

		void set_parameters(size_t maxiters_in, size_t curiter_in,
				size_t min_cand_in, size_t max_cand_in,
				size_t min_voters_in, size_t max_voters_in,
				bool show_median_in, stats_type br_type_in,
				list<pure_ballot_generator *> & generators_in,
				list<const election_method *> & methods_in);

		bayesian_regret(size_t maxiters_in, 
				size_t min_cand_in, size_t max_cand_in,
				size_t min_voters, size_t max_voters,
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
