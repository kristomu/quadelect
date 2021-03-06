// This mode takes user input and parses it into a set of ballots, after which
// it runs those ballots through the desired methods. The logic is to first
// check the input against every interpreter we have, then churn the ballots
// through the format that fits it well -- or force a certain format if the
// user desires so.

#ifndef _VOTE_MODE_INT
#define _VOTE_MODE_INT

#include "mode.h"

#include <map>
#include <list>
#include <vector>

#include "../ballots.h"
#include "../singlewinner/method.h"
#include "../interpreter/interpreter.h"

class interpreter_mode : public mode {

	private:
		vector<const election_method *> methods;
		list<const interpreter *> interpreters;
		list<ballot_group> input_ballots;
		vector<string> input_ballots_unparsed;
		list<ordering> results;
		map<size_t, string> cand_lookup;
		cache_map cache_inside;
		int cur_iter;
		bool inited, needs_interpreting;

		string base26(int number) const;
		void complete_default_lookup(map<size_t, string> & to_complete,
		    size_t how_many) const;
		map<size_t, string> gen_default_lookup(size_t how_many) const;

		// Get an upper bound on how many candidates we need to have
		// mapped.
		int get_max_candidates(const list<ballot_group> & in) const;

		// Interpret unparsed ballots. If it can't find any suitable
		// methods, it'll return false.
		// WARNING: Side effect! (May set input ballots and cand_lookup)
		bool parse_ballots(bool debug);

		void invalidate();

	public:
		void set_ballots(const list<ballot_group> & ballot_in);
		bool set_ballots(const list<ballot_group> & ballot_in,
		    const map<size_t, string> & candidate_names);
		// ... set_unparsed_ballots ...

		// Seen this before?
		void add_method(const election_method * to_add);
		void add_interpreter(const interpreter * to_add);
		template<typename T> void add_methods(T start_iter,
		    T end_iter);
		template<typename T> void add_interpreters(T start_iter,
		    T end_iter);

		void clear_methods(bool do_delete);
		void clear_interpreters(bool do_delete);

		interpreter_mode() {
			needs_interpreting = false;
			inited = false;
			cur_iter = 0;
		}
		interpreter_mode(list<const interpreter *> & interpreters_in,
		    list<const election_method *> & methods_in,
		    list<ballot_group> & ballots_in);
		interpreter_mode(list<const interpreter *> & interpreters_in,
		    list<const election_method *> & methods_in,
		    vector<string> & ballots_in_unparsed);
		interpreter_mode(list<const interpreter *> & interpreters_in,
		    list<const election_method *> & methods_in);

		// There really isn't much to do but init.
		bool init(rng & randomizer);

		int get_max_rounds() const;
		// 0 if nothing's going on.
		int get_current_round() const;

		string do_round(bool give_brief_status, bool reseed,
		    rng & randomizer, cache_map * cache);

		string do_round(bool give_brief_status, bool reseed,
		    rng & randomizer);

		vector<string> provide_status() const;
};

template<typename T> void interpreter_mode::add_methods(T start_iter,
    T end_iter) {

	for (T pos = start_iter; pos != end_iter; ++pos)
		add_method(*pos);

	inited = false;
}

template<typename T> void interpreter_mode::add_interpreters(T start_iter,
    T end_iter) {

	for (T pos = start_iter; pos != end_iter; ++pos)
		add_interpreter(*pos);

	if (!input_ballots_unparsed.empty() && input_ballots.empty())
		inited = false;
}


#endif
