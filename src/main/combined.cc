// All in one! The idea here is to build every possible simple method
// combination we can think of, then either dump it all so the user can specify
// which methods interest him, or intersect it with such a list winnowed down
// by the user. A more sophisticated version might try to parse the actual names
// so that we don't have to init every possible combination of methods 
// beforehand. (BLUESKY do that, which would thus permit things like LE/LE/LE/
// (Smith,Plurality).)

#include <assert.h>
#include <values.h>
#include <errno.h>
#include <ctype.h>

#include <getopt.h>

#include <iostream>
#include <fstream>
#include <list>

// General includes - tools and ballot structures.

#include "../ballot_tools.h"
#include "../ballots.h"
#include "../tools.h"

// Generators go here.

#include "../generator/all.h"

// Single-winner methods, including metamethods.

// DONE: Meta-header that includes all of these. 
// TODO? Perhaps move the "factory" itself into that, too.
#include "../singlewinner/all.h"

// Interpreters for the interpreter mode
#include "../interpreter/all.h"

// More later. POC for now. Perhaps also do something with the fact that C++
// doesn't have garbage collection -- i.e. clean up after ourselves.

// Headers for the Bayesian regret mode:
#include "../random/random.h"
#include "../stats/stats.h"

#include "../modes/breg.h"
#include "../modes/yee.h"

// Headers for the Yee mode:
#include "../images/color/color.h"
#include <openssl/sha.h>

// Interpreter itself.
#include "../modes/interpret.h"

list<pairwise_method *> get_pairwise_methods(
		const list<pairwise_type> & types, 
		bool include_experimental) {

	// For each type, and for each pairwise method, dump that combination
	// to the output. Possible feature request: have the method determine
	// if it supports the type in question (e.g. types that can give < 0
	// wrt stuff like Keener).

	list<pairwise_method *> out;

	for (list<pairwise_type>::const_iterator pos = types.begin(); pos !=
			types.end(); ++pos) {
		out.push_back(new dquick(*pos));
		out.push_back(new kemeny(*pos));
		out.push_back(new maxmin(*pos));
		out.push_back(new schulze(*pos));

		// Reasonable convergence defaults.
		out.push_back(new sinkhorn(*pos, 0.01, true));
		out.push_back(new sinkhorn(*pos, 0.01, false));
		out.push_back(new hits(*pos, 0.001));
		out.push_back(new odm_atan(*pos, 0.001));
		out.push_back(new odm_tanh(*pos, 0.001));
		out.push_back(new odm(*pos, 0.001));

		// Keener
		out.push_back(new keener(*pos, 0.001, false, false));
		out.push_back(new keener(*pos, 0.001, false, true));
		out.push_back(new keener(*pos, 0.001, true, false));
		out.push_back(new keener(*pos, 0.001, true, true));

		out.push_back(new ext_minmax(*pos, false));
		out.push_back(new ext_minmax(*pos, true));
		out.push_back(new ord_minmax(*pos));

		if (include_experimental) {
			out.push_back(new tup(*pos, TUP_TUP));
			out.push_back(new tup(*pos, TUP_SV));
			out.push_back(new tup(*pos, TUP_MIN));
			out.push_back(new tup(*pos, TUP_ALT_1));
			out.push_back(new tup(*pos, TUP_ALT_2));
			out.push_back(new tup(*pos, TUP_ALT_3));
			out.push_back(new tup(*pos, TUP_ALT_4));
			out.push_back(new tup(*pos, TUP_ALT_5));
			out.push_back(new tup(*pos, TUP_ALT_6));
		}
	}
	
	// Doesn't matter what these are set to. There should be a test for that
	// so that we can just do an if.
	
	// Defined elsewhere...
	//out.push_back(new copeland(CM_WV));

	out.push_back(new copeland(CM_WV, 2, 2, 1));
	out.push_back(new copeland(CM_WV, 2, 1, 0));
	out.push_back(new randpair(CM_WV));

	return(out);
}

// Positional methods.

list<positional *> get_positional_methods(bool truncate) {
	// Put that elsewhere?
	list<positional_type> types;
	if (truncate)
		for (int p = PT_FIRST; p <= PT_LAST; ++p)
			types.push_back(positional_type(p));
	// No point in using both if you aren't going to truncate.
	else	types.push_back(positional_type(PT_FRACTIONAL));
	
	list<positional *> out;

	for (list<positional_type>::const_iterator pos = types.begin(); pos !=
			types.end(); ++pos) {
		out.push_back(new plurality(*pos));
		out.push_back(new ext_plurality(*pos));
		out.push_back(new borda(*pos));
		out.push_back(new antiplurality(*pos));
		out.push_back(new ext_antiplurality(*pos));
		out.push_back(new for_and_against(*pos));
		out.push_back(new nauru(*pos));
		out.push_back(new heismantrophy(*pos));
		out.push_back(new baseballmvp(*pos));
		out.push_back(new eurovision(*pos));
		out.push_back(new dabagh(*pos));
		out.push_back(new nrem(*pos));
		out.push_back(new worstpos(*pos));
		out.push_back(new worstborda(*pos));
	}

	return(out);
}

list<pairwise_method *> get_sets() {

	list<pairwise_method *> out;

	out.push_back(new condorcet_set());
	out.push_back(new mdd_set(true));
	out.push_back(new mdd_set(false));
	out.push_back(new partition_set(false));

	out.push_back(new cdtt_set());
	out.push_back(new cgtt_set());
	out.push_back(new landau_set());
	out.push_back(new schwartz_set());
	out.push_back(new sdom_set());
	out.push_back(new pdom_set());
	out.push_back(new smith_set());

	out.push_back(new copeland(CM_WV)); // Nudge nudge.

	return(out);
}

template <typename T, typename Q> list<election_method *> expand_meta(
		const list<T *> & base_methods, const list<Q *> & sets,
		bool is_positional) {

	list<election_method *> kombinat;
	typename list<Q *>::const_iterator spos;

	for (typename list<T *>::const_iterator pos = base_methods.begin();
			pos != base_methods.end(); ++pos) {
		for (spos = sets.begin(); spos != sets.end(); ++spos) {
			if ((*pos)->name() == (*spos)->name()) continue;

			kombinat.push_back(new comma(*pos, *spos));
			// Use indiscriminately at your own risk! I'm not 
			// trusting this wholly until I can do hopefuls 
			// transparently.
			if (is_positional)
				kombinat.push_back(new slash(*pos, *spos));
		}
		// These are therefore constrained to positional 
		// methods for the time being. I think there may be bugs with
		// hopefuls for some of the advanced methods. I'm therefore
		// not doing anything with them, limiting LE and slash to 
		if (is_positional) {
			kombinat.push_back(new loser_elimination(*pos, true, 
						true));
			kombinat.push_back(new loser_elimination(*pos, false, 
						true));
		}
	}

	return(kombinat);
}


list<election_method *> get_singlewinner_methods(bool truncate, 
	bool include_experimental) {

	list<election_method *> toRet;

	list<pairwise_method *> pairwise = get_pairwise_methods(
			pairwise_producer().provide_all_strategies(),
			include_experimental);

	copy(pairwise.begin(), pairwise.end(), back_inserter(toRet));

	list<positional *> positional_methods = get_positional_methods(
			truncate);
	list<pairwise_method *> pairwise_sets = get_sets();

	// We have to do it in this clumsy manner because of possible bugs in
	// handling methods with some candidates excluded.
	list<election_method *> posnl_expanded = expand_meta(positional_methods,
			pairwise_sets, true);

	// Now add other methods here...
	copy(pairwise_sets.begin(), pairwise_sets.end(), back_inserter(toRet));

	// Gradual Condorcet-Borda with different bases, not just Condorcet.
	// The completion method doesn't really matter. Also, MDD* doesn't
	// really work here, and sets that can't handle negatives shouldn't
	// be applied to it.
	for (list<pairwise_method *>::const_iterator basis = pairwise_sets.
			begin(); basis != pairwise_sets.end(); ++basis) {
		toRet.push_back(new gradual_cond_borda(*basis, false, GF_BOTH));
		toRet.push_back(new gradual_cond_borda(*basis, true, GF_BOTH));
	}

	toRet.push_back(new young(true, true));
	toRet.push_back(new young(false, true));
	toRet.push_back(new first_pref_copeland());
	toRet.push_back(new random_ballot());
	toRet.push_back(new random_candidate());
	toRet.push_back(new cardinal_ratings(0, 10, false));
	toRet.push_back(new cardinal_ratings(0, 10, true));
	toRet.push_back(new mode_ratings());
	toRet.push_back(new vi_median_ratings(10, false, false));
	toRet.push_back(new vi_median_ratings(10, false, true));
	toRet.push_back(new vi_median_ratings(10, true, false));
	toRet.push_back(new vi_median_ratings(10, true, true));
	toRet.push_back(new dsc());

	if (include_experimental) {
		for (int i = 0; i < TEXP_TOTAL; ++i) {
			toRet.push_back(new three_experimental((texp_type)i));
		}

		toRet.push_back(new sv_att_second());
		toRet.push_back(new fpa_experiment());
	}

	// Then expand:
	list<election_method *> expanded = expand_meta(toRet, pairwise_sets, 
			false);

	// and

	copy(positional_methods.begin(), positional_methods.end(),
			back_inserter(toRet));
	copy(posnl_expanded.begin(), posnl_expanded.end(), 
			back_inserter(toRet));
	copy(expanded.begin(), expanded.end(), back_inserter(toRet));

	// Done!
	return(toRet);
}

// This should really be put into a file associated with all.h
list<pure_ballot_generator *> get_all_generators(bool compress, bool truncate) {

	list<pure_ballot_generator *> toRet;

	toRet.push_back(new dirichlet(compress, truncate));
	toRet.push_back(new gaussian_generator(compress, truncate));
	toRet.push_back(new impartial(compress, truncate));
	toRet.push_back(new dirichlet(compress, truncate));
	toRet.push_back(new uniform_generator(compress, truncate));

	return(toRet);
}

bayesian_regret setup_regret(list<election_method *> & methods, 
		list<pure_ballot_generator *> & generators,
		int maxiters, int min_candidates, int max_candidates,
		int min_voters, int max_voters, rng & randomizer) {

	// Do something with Bayesian regret here. DONE: Move over
	// to modes.

	list<const election_method *> rtmethods; // What a kludge.
	copy(methods.begin(), methods.end(), back_inserter(rtmethods));

	bayesian_regret br(maxiters, min_candidates, max_candidates,
			min_voters, max_voters, false, MS_INTRAROUND,
			generators, rtmethods);

	//rng randomizer(10); // <- or time

	assert(br.init(randomizer));

	return(br);

	/*string status;

	int counter = 0;

	cache_map cache;

	do {
		cache.clear();
		status = br.do_round(true, true, randomizer, cache);
		cout << status << endl;

		if (++counter % report_frequency == (report_frequency - 1)) {
			vector<string> report = br.provide_status();
			copy(report.begin(), report.end(), 
					ostream_iterator<string>(cout, "\n"));
		}
	} while (status != "");*/
}

// The hack with the generators is required so that C++ doesn't delete them 
// once the function is done.
yee setup_yee(list<election_method *> & methods, int num_voters, int num_cands,
		bool do_use_autopilot, string case_prefix, int picture_size,
		double sigma, gaussian_generator & gaussian, 
		uniform_generator & uniform, rng & randomizer) {

	yee to_output;

	assert(to_output.set_params(num_voters, num_cands, do_use_autopilot,
				case_prefix, picture_size, sigma));

	to_output.set_voter_pdf(&gaussian);
	to_output.set_candidate_pdf(&uniform);

	to_output.add_methods(methods.begin(), methods.end());

	assert(to_output.init(randomizer));

	return(to_output);
}

pair<bool, interpreter_mode> setup_interpreter(
		list<election_method *> & methods,
		list<const interpreter *> & interpreters,
		vector<string> & unparsed, rng & randomizer) {

	// Kludge, again!
	list<const election_method *> methods_const;
	copy(methods.begin(), methods.end(), back_inserter(methods_const));

	interpreter_mode toRet(interpreters, methods_const, unparsed);

	bool inited = toRet.init(randomizer);

	if (!inited) {
		cerr << "Interpreter mode error: Cannot parse ballot data!"
			<< endl;
		return(pair<bool, interpreter_mode>(false, toRet));
	}

	return(pair<bool, interpreter_mode>(true, toRet));
}

pair<bool, interpreter_mode> setup_interpreter_from_file(
		list<election_method *> & methods,
		list<const interpreter *> & interpreters,
		string file_name, rng & randomizer) {

	ifstream inf (file_name.c_str());

	if (!inf) {
		cerr << "Interpreter mode error: Could not open " <<
			file_name << " for reading!" << endl;
		return(pair<bool, interpreter_mode>(false, interpreter_mode()));
	}

	// Slurp the contents. Note, will crash the program if you feed it
	// /dev/zero or somesuch.

	vector<string> unparsed = slurp_file(inf, false);
	inf.close();

	return(setup_interpreter(methods, interpreters, unparsed, randomizer));
}

/// --- ///

template<typename T> void list_names(const T & container) {

	for (typename T::const_iterator pos = container.begin(); pos !=
			container.end(); ++pos)
		cout << (*pos)->name() << endl;
}

// Get only those that appear on the list.

string ignore_spaces_np(const string & a) {

	string toRet;

	for (size_t counter = 0; counter < a.size(); ++counter) {
		if (isprint(a[counter]) && a[counter] != ' ')
			toRet.push_back(a[counter]);
	}

	return(toRet);
}

template</*typename T,*/typename Q> list<Q> get_name_intersection(
		const list<Q> & container, const vector<string> accepted) {

	// We do this by first building a map of Q (it's presumed that T is
	// vector or list or something of Q), with Q's name as index. Then we
	// go through the accepted list and dump those that we find.

	map<string, Q> codebook;

	for (typename list<Q>::const_iterator pos = container.begin(); pos !=
			container.end(); ++pos)
		codebook[ignore_spaces_np((*pos)->name())] = *pos;

	list<Q> to_return;

	for (vector<string>::const_iterator spos = accepted.begin(); spos !=
			accepted.end(); ++spos) {
		typename map<string, Q>::const_iterator search =
			codebook.find(ignore_spaces_np(*spos));

		if (search != codebook.end())
			to_return.push_back(search->second);
	}

	return(to_return);
}

template<typename Q> list<Q> intersect_by_file(
		const list<Q> & container, string filename) {

	// Returns a container of NULL if we can't open.

	ifstream infile(filename.c_str());

	if (!infile) {
		cerr << "Cannot open " << filename << " for reading!" << endl;
		return(list<Q>(1, NULL));
	}

	vector<string> accepted = slurp_file(infile, true);
	infile.close();

	return(get_name_intersection(container, accepted));
}

void print_usage_info(string program_name) {

	cout << "Quadelect, election method analysis tool." << endl << endl;

	cout << "Usage: " << program_name << " [OPTIONS]... " << endl;
	cout << endl;

	cout << "General enumeration options:" << endl;
	cout << "\t-G\t\t List available ballot generators." << endl;
	// todo, implement!
	cout << "\t-I\t\t List available interpreters." << endl;
	cout << "\t-M\t\t List available election methods." << endl;
	cout << endl;
	cout << "Random number generator options:" << endl;
	cout << "\t-r [seed]\t Set the random number generator seed to [seed]."
		<< endl << endl;
	cout << "Constraint options: " << endl;
	cout << "\t-e\t\tEnable experimental methods. These are not intended for" <<
		"\n\t\t\tordinary use!" << endl;
	cout << "\t-g [file]\tUse the generators listed in [file]. The " <<
		"program will\n\t\t\tabort if that implies no generators " <<
		"are to be used." << endl;
	cout << "\t-ic [file]\tOnly use the interpreters listed in [file]."<<
		" The\n\t\t\tprogram will abort if that implies no interpreters"
		<<" are\n\t\t\tto be used." << endl;
	cout << "\t-m [file]\tOnly use the election methods listed in [file]."<<
		" The\n\t\t\tprogram will abort if that implies no methods " <<
		"are to be\n\t\t\tused." << endl;
	cout << endl;
	cout << "Bayesian regret calculation options:" << endl;
	cout << "\t-b\t\tEnable the Bayesian Regret calculation mode." << endl;
	cout << "\t-bi [maxiters]\tDo a maximum of [maxiters] rounds.\n\t\t\t"<<
		"Default is 20000." << endl;
	cout << "\t-bcm [cands]\tUse a minimum of [cands] candidates when"
		<< " performing a\n\t\t\tBayesian regret round. Default is 3." 
		<< endl;
	cout << "\t-bcx [cands]\tUse a maximum of [cands] candidates when"
		<< " performing a\n\t\t\tBayesian regret round. Default is 20."
		<< endl;
	cout << "\t-bvm [voters]\tUse a minimum of [voters] voters when"
		<< " performing a\n\t\t\tBayesian regret round. Default is 4." 
		<< endl;
	cout << "\t-bvx [voters]\tUse a maximum of [voters] voters when"
		<< " performing a\n\t\t\tBayesian regret round. Default is 200."
		<< endl;
	cout << "\t-brf [rounds]\tPrint Bayesian regret statistics every " <<
		"[rounds] rounds.\n\t\t\tDefault is 100." << endl;
	cout << endl;
	cout << "Interpreter (ballot counting) options: " << endl;
	cout << "\t-i\t\tEnable the interpreter/ballot counting mode." << endl;
	cout << "\t-if [file]\tUse [file] as the input file to interpret. The";
	cout << "\n\t\t\tdefault is \"interpret.txt\"." << endl;
	// todo, more here!
	cout << endl;
	cout << "Yee diagram calculation options:" << endl;
	cout << "\t-y\t\tEnable the Yee diagram calculation/rendering mode."
		<< endl;
	cout << "\t-ys [sigma]\tSet the standard deviation of the voter " <<
		"Gaussian\n\t\t\tto [sigma]. Default is 0.3" << endl;
	cout << "\t-yps [pixels]\tRender a [pixels]*[pixels] Yee diagram." <<
		"\n\t\t\tDefault is 240." << endl;
	cout << "\t-yna\t\tDon't use progressive sampling (\"autopilot\"). " <<
		"Disabling\n\t\t\tthis will turn the rendering slower." << endl;
	cout << "\t-yp [prefix]\tPrefix the rendered picture names by " <<
		"[prefix].\n\t\t\tDefault is \"default\"" << endl;
	cout << "\t-yv [voters]\tSample a maximum of [voters] voters per " <<
		"pixel.\n\t\t\tDefault is 1000." << endl;
	cout << "\t-yc [cands]\tPlace [cands] random candidates on the Yee"<<
		" map.\n\t\t\tDefault is 4." << endl;
}

int main(int argc, char * * argv) {

	// Set parameter defaults.

	double yee_sigma = 0.3;
	int yee_size = 240, yee_voters = 1000, yee_candidates = 4;
	bool yee_autopilot = true;
	string yee_prefix = "default";

	int breg_rounds = 20000, breg_min_cands = 3, breg_max_cands = 20,
	    breg_min_voters = 4, breg_max_voters = 200, breg_report_freq = 100;

	bool run_yee = false, run_breg = false, run_int = false,
	     list_methods = false, list_gen = false, list_int = false, 
	     use_exp_averaging = false;
	int run_how_many = 0;

	bool constrain_methods = false, constrain_generators = false,
	     constrain_ints = false;

	bool include_experimental = false;

	string method_constraint_fn, generator_constraint_fn, int_constraint_fn;

	string int_source_file = "interpret.txt";

	rng randomizer(get_abs_time());
	uint64_t seed = randomizer.long_rand();

	bool success = true;

	string ext;

	static struct option long_options[] = {
		// Long options (we'll do the rest after this struct).
		// We can't use these letters: eMGmgby

		// The first here is dummy because its option_index is 0.
		// Thus it can't be reached and only exists so the others
		// can be.
		{"\0?", no_argument,       0, '?'},
		{"ic", required_argument, 0, 'p'},
		{"if", required_argument, 0, 'q'},
		{"bi", required_argument, 0, 'a'},
		{"bcm", required_argument, 0, 'c'},
		{"bcx", required_argument, 0, 'd'},
		{"bvm", required_argument, 0, 'e'},
		{"bvx", required_argument, 0, 'f'},
		{"brf", required_argument, 0, 'g'},
		{"ys", required_argument, 0, 'h'},
		{"yps", required_argument, 0, 'o'},
		{"yna", no_argument, 0, 'j'},
		{"yp", required_argument, 0, 'k'},
		{"yv", required_argument, 0, 'l'},
		{"yc", required_argument, 0, 'n'},
		{0, 0, 0, 0}
	};

	int c, option_index = 0, opt_flag;

	while ((c = getopt_long_only(argc, argv, "ebyMGIim:g:r:",
				long_options, &option_index)) != -1) {

		if (option_index != 0)
			c = 0; // Access the long options!

		if (optarg)
			ext = optarg;

		switch(c) {
			case 0:
				opt_flag = long_options[option_index].val;
				option_index = 0; // Don't confuse later params.

				// We go here if it's one of the long options.
				switch(opt_flag) {
					case 'a': // -bi
						breg_rounds = str_toi(ext);
						assert(breg_rounds > 0);
						break;
					case 'c': // -bcm  mincand
						breg_min_cands = str_toi(ext);
						assert (breg_min_cands > 1);
						break;
					case 'd': // -bcx  maxcand
						breg_max_cands = str_toi(ext);
						assert (breg_max_cands > 0);
						break;
					case 'e': // -bvm  minvoter
						breg_min_voters = str_toi(ext);
						assert (breg_min_voters > 0);
						break;
					case 'f': // -bvx  maxvoter
						breg_max_voters = str_toi(ext);
						assert (breg_max_voters > 0);
						break;
					case 'g': // -brf  report_freq
						breg_report_freq = str_toi(ext);
						assert (breg_report_freq > 0);
						break;
					case 'h': // -ys   sigma
						yee_sigma = str_tod(ext);
						assert (yee_sigma > 0);
						break;
					case 'o': // -yps  picsize
						yee_size = str_toi(ext);
						assert(yee_size > 0);
						break;
					case 'j': // -yna  autopilot
						yee_autopilot = false;
						break;
					case 'k': // -yp   prefix
						yee_prefix = ext;
						break;
					case 'l': // -yv   voters
						yee_voters = str_toi(ext);
						assert (yee_voters > 0);
						break;
					case 'n': // -yc   candidates
						yee_candidates = str_toi(ext);
						assert (yee_candidates > 1);
						break;
					case 'p': // -ic [filename]
						int_constraint_fn = ext;
						constrain_ints = true;
						break;
					case 'q': // -if [filename]
						int_source_file = ext;
						break;
					case '?':
						success = false;
						break;
				}
				break;
			case 'e': // enable experimental
				include_experimental = true;
				break;
			case 'y': // do Yee
				run_yee = true;
				++run_how_many;
				break;
			case 'b': // do Bayesian regret
				run_breg = true;
				++run_how_many;
				break;
			case 'i': // do parse external ballot data.
				run_int = true;
				++run_how_many;
				break;
			case 'M': // print election methods.
				list_methods = true;
				++run_how_many;
				break;
			case 'G':
				list_gen = true;
				++run_how_many;
				break;
			case 'I':
				list_int = true;
				++run_how_many;
				break;
			case 'm':
				 constrain_methods = true;
				 method_constraint_fn = ext;
				 break;
			case 'g':
				 constrain_generators = true;
				 generator_constraint_fn = ext;
				 break;
			case 'r': // -r: set random seed.
				 seed = str_toull(ext);
				 break;
			default:
				// If it's a long option, don't bother with it.
				if (option_index != 0)
					option_index = 0;
				else {
					/*cout << "I pity the foo. " << (char)c 
						<< endl;*/
					success = false;
				}
		}
	}

	if (run_how_many > 1) {
		cerr << argv[0] << ": more than one action specified. Please "
			<< "be more specific." << endl;
		return(-1);
	}

	// If he made a booboo with the parameters or if he didn't specify
	// anything to do, print usage information and get outta here.
	if (!success || run_how_many == 0) {
		print_usage_info(argv[0]);
		return(-1);
	}

	// Actions ahoy!
	// TODO: Let user specify truncation. Do so when truncation is actually
	// consistent with the logic of the generators instead of just being
	// something that's slapped on.

	list<election_method *> methods = get_singlewinner_methods(false,
		include_experimental);
	list<election_method *>::const_iterator pos;

	list<pure_ballot_generator *> generators = get_all_generators(true, 
			false);

	list<pure_ballot_generator *> default_br_generators;
	default_br_generators.push_back(new uniform_generator(true, false));

	// These are used so it'll be possible to dealloc early if we so want.
	// TODO later: have intersect provide two arrays: one for intersection,
	// one for the rest. Then just free those in the "rest" category.
	list<election_method *> methods_backup = methods;
	list<pure_ballot_generator *> generators_backup = generators;

	// Interpreters for the interpreter mode
	list<interpreter *> interpreters = get_all_interpreters();

	if (list_gen) {
		list_names(generators);
		return(0);
	}

	if (list_methods) {
		list_names(methods); // Aint templating grand?
		return(0);
	}

	if (list_int) {
		list_names(interpreters);
		return(0);
	}

	// Constraining goes here.

	if (constrain_methods) {
		methods = intersect_by_file(methods, method_constraint_fn);
		if (methods.empty()) {
			cerr << argv[0] << ": Found no eligible method names in"
				<< " " << method_constraint_fn << endl;
			return(-1);
		}
		if (*(methods.begin()) == NULL)
			return(-1); // couldn't open file.

		cout << "Method constraint: loaded " << methods.size() 
			<< " methods." << endl;
	}

	if (constrain_generators) {
		generators = intersect_by_file(generators, 
				generator_constraint_fn);
		if (generators.empty() || *generators.begin() == NULL) {
			cerr << argv[0] << ": Found no eligible method names in"
				<< " " << generator_constraint_fn << endl;
			return(-1);
		}

		// If we get here, we have constrained, so se the default breg.
		// Dealloc the one we had first, though.
		delete *(default_br_generators.begin());

		default_br_generators = generators;

		cout << "Generator constraint: loaded " << generators.size()
			<< " generators." << endl;
	}

	if (constrain_ints) {
		interpreters = intersect_by_file(interpreters,
				int_constraint_fn);
		if (interpreters.empty() || *interpreters.begin() == NULL) {
			cerr << argv[0] << ": Found no eligible interpreter " <<
				"names in " << int_constraint_fn << endl;
			return(-1);
		}

		cout << "Interpreter constraint: loaded " << interpreters.size()
			<< " interpreters." << endl;
	}

	cout << "Random number generator: using seed " << seed << endl;
	randomizer.s_rand(seed);

	// Run_yee and Run_breg goes here.
	// You need to prepare all the right stuff and then call either yee
	// or breg. After it's done, just use the same loop since they're both
	// instances of the ABC, mode.

	yee yee_mode;
	bayesian_regret br_mode;
	pair<bool, interpreter_mode> int_mode;
	mode * mode_running;

	uniform_generator uniform(true, false);
	gaussian_generator gaussian(true, false);

	if (run_yee) {
		cout << "Setting up Yee..." << endl;
		cout << "\t\t- sigma: " << yee_sigma << endl;
		cout << "\t\t- picture size: " << yee_size << endl;
		cout << "\t\t- max number of voters: " << yee_voters << endl;
		cout << "\t\t- number of candidates: " << yee_candidates << endl;
		cout << "\t\t- autopilot: ";
		if (yee_autopilot)
			cout << "yes" << endl;
		else	cout << "no" << endl;
		cout << "\t\t- picture prefix: " << yee_prefix << endl;

		if (methods.size() > 10) {
			cout << "WARNING: You have selected more than 10 " <<
				"methods at once. This may take a *very* " <<
				"long time." << endl;
		}

		yee_mode = setup_yee(methods, yee_voters, yee_candidates, 
				yee_autopilot, yee_prefix, yee_size, yee_sigma,
				gaussian, uniform, randomizer);

		mode_running = &yee_mode;

		// Yee diagrams with autopilot takes a lot longer time in the
		// middle than in the beginning, so we should use an exponential
		// average. Better might be to draw the lines in random order.
		use_exp_averaging = true;
	}

	if (run_breg) {
		cout << "Setting up Bayesian Regret..." << endl;
		cout << "\t\t- rounds: " << breg_rounds << endl;
		cout << "\t\t- minimum number of candidates: " << breg_min_cands
			<< endl;
		cout << "\t\t- maximum number of candidates: " << breg_max_cands
			<< endl;
		cout << "\t\t- minimum number of voters: " << breg_min_voters
			<< endl;
		cout << "\t\t- maximum number of voters: " << breg_max_voters
			<< endl;
		cout << "\t\t- report frequency: " << breg_report_freq << endl;

		br_mode = setup_regret(methods, default_br_generators,
				breg_rounds, breg_min_cands, breg_max_cands,
				breg_min_voters, breg_max_voters, randomizer);

		mode_running = &br_mode;

		// Bayesian regret calculations are fairly consistent as far as
		// time is concerned (has no time-depending factor), but it can
		// oscillate wildly, particularly with methods like Kemeny or
		// Yee that can take a very long time in the worst case. Thus,
		// we shouldn't use exponential averaging.
		use_exp_averaging = false;
	}

	if (run_int) {
		cout << "Setting up ballot interpretation..." << endl;
		cout << "\t\t- input file: " << int_source_file << endl;
		cout << "\t\t- number of interpreters: " << interpreters.size()
			<< endl;
		cout << "\t\t- number of methods: " << methods.size() << endl;

		list<const interpreter *> int_temp; // yay conversion!
		for (list<interpreter *>::const_iterator cpos = 
				interpreters.begin(); 
				cpos != interpreters.end(); ++cpos)
			int_temp.push_back(*cpos);

		int_mode = setup_interpreter_from_file(methods, int_temp,
				int_source_file, randomizer);
		
		if (!int_mode.first)
			return(-1); // Something wrong, outta here.

		mode_running = &int_mode.second;

		use_exp_averaging = false;
	}

	string progress;
        double start_time = get_abs_time(), cur_checkpoint = start_time,
	       cur_disp_checkpoint = start_time;
	double per_round = -1;
	
    while ( (progress = mode_running->do_round(true, false, randomizer)) 
		!= "") {
		cout << progress << endl;

		double this_instance = (get_abs_time() - cur_checkpoint);

		if (per_round < 0)  {
			per_round = this_instance;
		} else {
			// Change more quickly if there are few values backing
			// the result.
			double factor = max(0.03, 1 / (double)(mode_running->
						get_current_round()));
			// Exponential averaging for ETA.
			per_round = per_round * (1-factor) + this_instance *
				factor;
		}

		cur_checkpoint = get_abs_time();

		if (get_abs_time() - cur_disp_checkpoint > 2) {
			double elapsed_time = get_abs_time()-start_time;

			if (!use_exp_averaging)
				per_round = elapsed_time / (double)
					mode_running->get_current_round();

			cout << "\t [Has run for " << elapsed_time
				<< "s. ETA: " << per_round * (mode_running->
						get_max_rounds()-mode_running->
						get_current_round()) << "s, "<<
				" for a total of " <<
				per_round * mode_running->get_max_rounds() << 
				" s. ]" << endl;

			cur_disp_checkpoint = cur_checkpoint;
		}

		bool should_display_stats = false;
		// If we're at the last round or if at the breg_report_freq and
		// Bayesian regret, report.

		if (mode_running->get_current_round() == 
				mode_running->get_max_rounds())
			should_display_stats = true;
		else	should_display_stats = run_breg && 
				mode_running->get_current_round() % 
				breg_report_freq == (breg_report_freq-1);
		
		if (should_display_stats) {
			vector<string> report = mode_running->provide_status();
			copy(report.begin(), report.end(), ostream_iterator<
					string>(cout, "\n"));
			cout << endl;
		}
	}
}
