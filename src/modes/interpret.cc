// This mode takes user input and parses it into a set of ballots, after which
// it runs those ballots through the desired methods. The logic is to first
// check the input against every interpreter we have, then churn the ballots
// through the format that fits it well -- or force a certain format if the
// user desires so.

#include "interpret.h"
#include "../tools/ballot_tools.h"

// Functions for generating standard candidate names.

std::string interpreter_mode::base26(int number) const {

	std::string output;

	do {
		int remnant = number % 26;
		output = (char)(remnant + 'A') + output;
		number = (number - remnant) / 26;
	} while (number > 0);

	return (output);
}

void interpreter_mode::complete_default_lookup(
	std::map<size_t, std::string> &
	to_complete,
	size_t how_many) const {

	for (size_t counter = 0; counter < how_many; ++counter)
		if (to_complete.find(counter) == to_complete.end()) {
			to_complete[counter] = "<" + base26(counter) + ">";
		}
}

std::map<size_t, std::string> interpreter_mode::gen_default_lookup(
	size_t how_many) const {
	std::map<size_t, std::string> toRet;

	complete_default_lookup(toRet, how_many);

	return (toRet);
}

// Get an upper bound on how many candidates we need to have mapped. The logic
// is that if there's a rank of k candidates, we're going to need at least k
// candidate names; and if there's a score involving candidate q, we're going
// to need at least q.

int interpreter_mode::get_max_candidates(const std::list<ballot_group> &
	in)
const {
	size_t maxval = 0;

	for (std::list<ballot_group>::const_iterator pos = in.begin(); pos !=
		in.end(); ++pos) {
		maxval = std::max(maxval, pos->contents.size());

		for (ordering::const_iterator opos = pos->contents.begin();
			opos != pos->contents.end(); ++opos) {
			maxval = std::max(maxval, (size_t)opos->get_candidate_num());
		}
	}

	return (maxval);
}

bool interpreter_mode::parse_ballots(bool debug) {

	if (input_ballots_unparsed.empty()) {
		return (false);
	}

	const interpreter * to_use = NULL;

	// Try to find an interpreter.
	for (std::list<const interpreter *>::const_iterator cand_int =
			interpreters.begin(); cand_int != interpreters.end() &&
		to_use == NULL;	++cand_int)
		if ((*cand_int)->is_this_format(input_ballots_unparsed)) {
			to_use = *cand_int;
		}

	// If we didn't find any , bail.
	if (to_use == NULL) {
		return (false);
	}

	// Otherwise, let's get going!

	std::pair<std::map<size_t, std::string>, std::list<ballot_group> > parsed =
		to_use->
		interpret_ballots(input_ballots_unparsed, debug);

	if (parsed.second.empty()) {
		return (false);
	}

	// Put them where they belong.
	complete_default_lookup(parsed.first, get_max_candidates(parsed.
			second));
	set_ballots(parsed.second, parsed.first);
	needs_interpreting = false;

	return (true);
}

void interpreter_mode::invalidate() {
	inited = false;
	cur_iter = -1;
	//cur_iter_ptr = methods.end();
}

// Add ballots we want to count.
void interpreter_mode::set_ballots(const std::list<ballot_group> &
	ballot_in) {

	invalidate();
	input_ballots = ballot_in; // Simple enough.

	// Now count how many candidates we have so we can generate enough
	// default lookups.

	int maxval = get_max_candidates(ballot_in);

	// Then make as many default candidates as we need.
	cand_lookup = gen_default_lookup(maxval);
}

bool interpreter_mode::set_ballots(const std::list<ballot_group> &
	ballot_in,
	const std::map<size_t, std::string> & candidate_names) {

	// First add the ballots themselves. Then check if the needed map to
	// cover every candidate is larger than the one we've got provided. If
	// so, it's not applicable, so do the best we can and return false.
	// (Could be optimized if so desired, but you know what they say about
	// premature optimization.)

	invalidate();
	input_ballots = ballot_in;
	cand_lookup = candidate_names;

	size_t maxval = get_max_candidates(ballot_in);

	if (maxval > candidate_names.size()) {
		// User can still proceed, but with default canddt names
		// added in.
		complete_default_lookup(cand_lookup, maxval);
		return (false);
	}

	return (true);
}

// Second verse, same as the first! I am methody the eight, I am...
void interpreter_mode::add_method(const election_method * to_add) {
	methods.push_back(to_add);
}

void interpreter_mode::add_interpreter(const interpreter * to_add) {
	interpreters.push_back(to_add);
}

void interpreter_mode::clear_methods(bool do_delete) {

	if (do_delete)
		for (std::vector<const election_method *>::iterator pos =
				methods.begin(); pos != methods.end();
			++pos) {
			delete *pos;
		}

	methods.clear();
	invalidate();
}

void interpreter_mode::clear_interpreters(bool do_delete) {

	if (do_delete)
		for (std::list<const interpreter *>::iterator pos =
				interpreters.begin(); pos != interpreters.end();
			++pos) {
			delete *pos;
		}

	interpreters.clear();
	invalidate();
}

interpreter_mode::interpreter_mode(std::list<const interpreter *> &
	interpreters_in,
	std::list<const election_method *> & methods_in,
	std::list<ballot_group> & ballots_in) {

	interpreters = interpreters_in;
	add_methods(methods_in.begin(), methods_in.end());
	input_ballots = ballots_in;
	inited = false;
	needs_interpreting = false;

}

interpreter_mode::interpreter_mode(std::list<const interpreter *> &
	interpreters_in,
	std::list<const election_method *> & methods_in,
	std::vector<std::string> & ballots_in_unparsed) {

	interpreters = interpreters_in;
	add_methods(methods_in.begin(), methods_in.end());
	input_ballots_unparsed = ballots_in_unparsed;
	needs_interpreting = true;
	inited = false;
}

interpreter_mode::interpreter_mode(std::list<const interpreter *> &
	interpreters_in,
	std::list<const election_method *> & methods_in) {

	interpreters = interpreters_in;
	add_methods(methods_in.begin(), methods_in.end());
	needs_interpreting = false;
	inited = false;
}

bool interpreter_mode::init(rng & randomizer) {

	bool debug = false;

	// Do some checks. If there are no methods, fail.
	if (methods.empty()) {
		return (false);
	}

	// If we need to interpret something... and there are no interpreters,
	// fail.
	if (needs_interpreting) {
		// ... and there are no interpreters, fail.
		if (interpreters.empty()) {
			return (false);
		}
		// ... and there are no interpreters we can actually use, fail.
		if (!parse_ballots(debug)) {
			return (false);
		}
	}

	// If there's nothing to check, fail again!
	if (input_ballots.empty()) {
		return (false);
	}

	// In any case, just to be sure, complete the candidate names.
	complete_default_lookup(cand_lookup, get_max_candidates(input_ballots));
	// Also clear the list of results.
	results.clear();

	// Okay, we're ready to go.
	inited = true;
	cur_iter = 0;
	//cur_iter_ptr = methods.begin();
	return (true);

}

int interpreter_mode::get_max_rounds() const {
	return (methods.size());
}

// 0 if nothing's going on.
int interpreter_mode::get_current_round() const {
	if (!inited) {
		return (0);
	} else	{
		return (cur_iter);
	}
}

std::string interpreter_mode::do_round(bool give_brief_status, bool reseed,
	rng & randomizer, cache_map * cache) {

	// If we aren't inited, return to tell them no go.
	if (!inited) {
		return ("");
	}

	if (cur_iter >= get_max_rounds()) {
		return ("");    // All done, so signal it.
	}

	if (reseed) {
		randomizer.s_rand(cur_iter);
	}

	std::string output = "OK";

	// Just elect and increment. If we get here, the lookup map will already
	// have been completed, so we can use its size for number of candidates.
	int numcands = cand_lookup.size();
	ordering next = methods[cur_iter]->elect(input_ballots, numcands,
			cache, true);

	if (give_brief_status)
		output = "Done getting results for " + methods[cur_iter]->
			name();

	// Increment.
	++cur_iter;

	// And add the result to our list of results.
	results.push_back(next);

	return (output);
}

std::string interpreter_mode::do_round(bool give_brief_status, bool reseed,
	rng & randomizer) {
	return (do_round(give_brief_status, reseed, randomizer, NULL));
}

// More results than status, so more in the vein of the Bayesian Regret mode
// than the Yee mode.
std::vector<std::string> interpreter_mode::provide_status() const {

	if (!inited) {
		return (std::vector<std::string>(1, "Not inited!"));
	}

	std::vector<std::string> toRet;

	std::list<ordering>::const_iterator opos = results.begin();
	std::vector<const election_method *>::const_iterator empos =
		methods.begin();

	ordering_tools otool;

	// TODO: Whether or not to have numeric - based both on user param and
	// on whether the method returns numeric values.

	while (opos != results.end() && empos != methods.end()) {
		std::string this_result = "Result for " + (*empos)->name() + " : " +
			otool.ordering_to_text(*opos, cand_lookup, false);
		toRet.push_back(this_result);
		++opos;
		++empos;
	}

	return (toRet);
}
