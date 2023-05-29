// Possible BLUESKY: Support different utility aggregation functions. Mean,
// median, interquartile range, minimum, maximum, mean with Gini penalty, etc...

// Hm, have some kind of logic where if you add a method, it resets? Some way
// of signaling this?

// Possibly change this by removing the MS_* type that we don't need.

#include "breg.h"
#include <stdexcept>

bayesian_regret::bayesian_regret() {

	// Set some reasonable defaults.

	inited = false;
	maxiters = 100; curiter = 0;
	min_candidates = 2; max_candidates = 16;
	min_voters = 2; max_voters = 128;
	show_median = false; br_type = MS_INTRAROUND;
}

// Use clear_curiters if you want to run a new round.
void bayesian_regret::set_maxiters(size_t maxiters_in,
	bool clear_curiters) {

	maxiters = maxiters_in;

	if (clear_curiters) {
		curiter = 0;
	} else if (curiter >= maxiters) {
		curiter = maxiters - 1; // Means it's done.
	}
}

void bayesian_regret::set_num_candidates(size_t min, size_t max) {

	if (min > max) {
		throw std::range_error("bayesian_regret: min cand > max cand");
	}

	min_candidates = min;
	max_candidates = max;
}

void bayesian_regret::set_num_voters(size_t min, size_t max) {

	if (min > max) {
		throw std::range_error("bayesian_regret: min voters > max voters");
	}

	min_voters = min;
	max_voters = max;
}

void bayesian_regret::set_format(bool do_show_median) {
	show_median = do_show_median;
}

void bayesian_regret::set_br_type(const stats_type br_type_in) {
	br_type = br_type_in;

	if (inited) {
		inited = false;
		method_stats.clear();
	}
}

void bayesian_regret::add_generator(pure_ballot_generator * to_add) {
	generators.push_back(to_add);
}

void bayesian_regret::add_method(const election_method * to_add) {
	methods.push_back(to_add);

	// We'll let the user add methods. in the middle of things if he
	// so desiers. Caveat emptor - he'll have to be sure to check the
	// confidence interval so as to know they haven't run the same number
	// of trials, though!
	if (inited) {
		init_one(methods.size()-1);
	}
}

void bayesian_regret::clear_generators(bool do_delete) {

	if (do_delete) {

		for (size_t counter = 0; counter < generators.size(); ++counter) {
			delete generators[counter];
		}
	}

	generators.resize(0);
	inited = false;
}

void bayesian_regret::clear_methods(bool do_delete) {

	if (do_delete)
		for (size_t counter = 0; counter < methods.size(); ++counter) {
			delete methods[counter];
		}

	methods.clear();
	method_stats.clear();
	inited = false;
}

void bayesian_regret::set_parameters(size_t maxiters_in, size_t curiter_in,
	size_t min_cand_in, size_t max_cand_in, size_t min_voters_in,
	size_t max_voters_in, bool show_median_in, stats_type br_type_in,
	list<pure_ballot_generator *> & generators_in,
	list<const election_method *> & methods_in) {

	curiter = curiter_in;
	set_maxiters(maxiters_in, false);
	set_num_candidates(min_cand_in, max_cand_in);
	set_num_voters(min_voters_in, max_voters_in);

	set_format(show_median_in);
	set_br_type(br_type_in);

	clear_generators(false);
	clear_methods(false);

	copy(generators_in.begin(), generators_in.end(), back_inserter(
			generators));
	copy(methods_in.begin(), methods_in.end(), back_inserter(methods));
	inited = false;
}

bayesian_regret::bayesian_regret(size_t maxiters_in, size_t min_cand_in,
	size_t max_cand_in, size_t min_voters, size_t max_voters,
	bool show_median_in, stats_type br_type_in,
	list<pure_ballot_generator *> & generators_in,
	list<const election_method *> & methods_in) {

	inited = false;
	set_parameters(maxiters_in, 0, min_cand_in, max_cand_in, min_voters,
		max_voters, show_median_in, br_type_in, generators_in,
		methods_in);
}

bool bayesian_regret::init_one(size_t idx) {

	if (idx <= method_stats.size() && idx >= methods.size()) {
		return (false);
	}

	// Because stats come in a linear order, handle previous methods
	// before this one if there's a gap.
	if (idx > method_stats.size())
		if (!init_one(idx-1)) {
			return (false);
		}

	// Okay, we now know that idx is either inside the array of stats
	// already allocated, or just after it. If it's after, add another
	// entry; if not, overwrite the one that's already there.

	if (idx == method_stats.size()) {
		method_stats.push_back(stats<float>(br_type,
				methods[idx]->name(), false));
	} else {
		method_stats[idx] = stats<float>(br_type, methods[idx]->name(),
				false);
	}

	return (true);
}

bool bayesian_regret::init(rng & randomizer) {

	curiter = 0;
	method_stats.clear();
	inited = false;

	if (init_one(methods.size()-1)) {
		inited = true;
	}

	return (inited);
}

string bayesian_regret::do_round(bool give_brief_status, bool reseed,
	rng & randomizer, cache_map * cache) {

	if (curiter >= maxiters) {
		return ("");    // All done, so signal it.
	}

	string toRet_denied = "", toRet = "OK";

	if (reseed) {
		randomizer.s_rand(curiter);
	}

	int numcands = randomizer.irand(min_candidates, max_candidates+1);
	int numvoters = randomizer.irand(min_voters, max_voters+1);

	if (give_brief_status)
		toRet = "Now going on " + dtos(curiter) + " with " +
			dtos(numcands) + " cands and " + dtos(numvoters) +
			" voters.";

	// This lets the test use multiple generators, whereupon they are each
	// used (numrounds)/(num_generators) times.
	list<ballot_group> ballots = generators[curiter % generators.size()]->
		generate_ballots(numvoters, numcands, randomizer);

	// Get the utility scores, maximum and minimum. (TODO?? If we're
	// checking medians, shouldn't this be median ratings instead? etc. for
	// other modes).
	cardinal_ratings utility(MININT, MAXINT, false);
	ordering out = utility.elect(ballots, numcands, cache, false);

	double maxval = out.begin()->get_score(),
		   minval = out.rbegin()->get_score();

	// Put all the candidate scores in a vector format so we can easily
	// access the utility of the candidate that won in each method.
	ordering::const_iterator opos;
	utilities.resize(numcands);

	for (opos = out.begin(); opos != out.end(); ++opos) {
		utilities[opos->get_candidate_num()] = opos->get_score();
	}

	// Get the winner/s and Bayesian regret for each method.

	vector<stats<float> >::iterator mstat_pos = method_stats.begin();

	for (vector<const election_method *>::const_iterator pos =
			methods.begin(); pos != methods.end(); ++pos) {
		out = (*pos)->elect(ballots, numcands, cache, true);

		// Determine the utility. If it's a tie, the utility is
		// just the candidates' mean. (That, too, will have to be
		// altered if we use other utility aggregation mechanisms.)

		double numerator = 0, denominator = 0;
		for (opos = out.begin(); opos != out.end() &&
			opos->get_score() == out.begin()->get_score();
			++opos) {
			++denominator;
			numerator += utilities[opos-> get_candidate_num()];
		}

		// Because it's regret, the minimum value is actually
		// the maximum utility, and the maximum value is the
		// minimum utility.

		mstat_pos->add_result(maxval, numerator/ denominator,
			minval);

		/*if (counter % report_frequency == (report_frequency-1))
			cout << mstat_pos->display_stats(false, 0.05)
				<< endl;*/

		++mstat_pos;
	}

	++curiter;

	return (toRet);
}

string bayesian_regret::do_round(bool give_brief_status, bool reseed,
	rng & randomizer) {
	cache_map cache;

	return (do_round(give_brief_status, reseed, randomizer, &cache));
}

vector<string> bayesian_regret::provide_status() const {

	// If we aren't inited, then there's nothing we can do.

	if (!inited) {
		return (vector<string>());
	}

	// Otherwise, just dump the stats' info. TODO: some way of specifying
	// the confidence interval. For now, just use 0.05 (95%).

	vector<string> information;

	for (vector<stats<float> >::const_iterator pos = method_stats.begin();
		pos != method_stats.end(); ++pos)
		information.push_back(pos->display_stats(show_median,
				0.05));

	return (information);
}
