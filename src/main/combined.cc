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

list<pairwise_method *> get_pairwise_methods(
		const list<pairwise_type> & types) {

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
		out.push_back(new odm(*pos, 0.001));

		out.push_back(new ext_minmax(*pos, false));
		out.push_back(new ext_minmax(*pos, true));
		out.push_back(new ord_minmax(*pos));
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
		out.push_back(new borda(*pos));
		out.push_back(new antiplurality(*pos));
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


list<election_method *> get_singlewinner_methods(bool truncate) {

	list<election_method *> toRet;

	list<pairwise_method *> pairwise = get_pairwise_methods(
			pairwise_producer().provide_all_strategies());

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
	toRet.push_back(new random_ballot());
	toRet.push_back(new random_candidate());
	toRet.push_back(new cardinal_ratings(0, 10, false));
	toRet.push_back(new cardinal_ratings(0, 10, true));
	toRet.push_back(new mode_ratings());
	toRet.push_back(new vi_median_ratings(10, false, false));
	toRet.push_back(new vi_median_ratings(10, false, true));
	toRet.push_back(new vi_median_ratings(10, true, false));
	toRet.push_back(new vi_median_ratings(10, true, true));

	// The following methods only support three candidates or less,
	// Thus I've commmented them out for the time being.
	// BLUESKY: Make the selected routine somehow aware of how many 
	// candidates there are, so that it just excludes use these when
	// working with the wrong number of candidates.
	//toRet.push_back(new cond_brute(97603, 9)); // strategy-resistant method
	//toRet.push_back(new cond_brute_rpn(56182650307)); // example monotone
	//toRet.push_back(new three_experimental(TEXP_BPW)); // Stensholt method

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
	toRet.push_back(new uniform_generator(compress, truncate));

	return(toRet);
}

bayesian_regret setup_regret(list<election_method *> & methods, 
		list<pure_ballot_generator *> & generators,
		int maxiters, int min_candidates, int max_candidates,
		int min_voters, int max_voters, rng & randomizer) {

	// Do something with Bayesian regret here. DONE: Move over
	// to modes.

	/*list<pure_ballot_generator *> generators;
	generators.push_back(new uniform_generator(true, false));
	generators.push_back(new impartial(true, false));*/

	/*int maxiters = 40000;
	int min_candidates = 4, max_candidates = 20;
	int min_voters = 3, max_voters = 200;*/

	//int report_frequency = 200;

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

	vector<string> accepted;
	string next;

	while (!infile.eof()) {
		getline(infile, next);
		if (!infile.eof()) {
			cout << next << endl;
			accepted.push_back(next);
		}
	}

	infile.close();

	return(get_name_intersection(container, accepted));
}

void print_usage_info(string program_name) {

	cout << "Quadelect, election method analysis tool." << endl << endl;

	cout << "Usage: " << program_name << " [OPTIONS]... " << endl;
	cout << endl;

	cout << "General enumeration options:" << endl;
	cout << "\t-M\t\t List available election methods." << endl;
	cout << "\t-G\t\t List available ballot generators." << endl;
	cout << endl;
	cout << "Constraint options: " << endl;
	cout << "\t-m [file]\tOnly use the election methods listed in [file]."<<
		" The\n\t\t\tprogram will abort if that implies no methods " <<
		"are to be\n\t\t\tused." << endl;
	cout << "\t-g [file]\tUse the generators listed in [file]. The " <<
		"program will\n\t\t\tabort if that implies no generators " <<
		"are to be used." << endl;
	cout << endl;
	cout << "Bayesian regret calculation options:" << endl;
	cout << "\t-b\t\tEnable the Bayesian Regret calculation mode." << endl;
	cout << "\t-bi [maxiters]\tDo a maximum of [maxiters] rounds.\n\t\t"<< 
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

	bool run_yee = false, run_breg = false, list_methods = false, 
	     list_gen = false;
	int run_how_many = 0;

	bool constrain_methods = false, constrain_generators = false;
	string method_constraint_fn, generator_constraint_fn;

	bool success = true;

	string ext;

	// None of these seem to work! Find out why.
	static struct option long_options[] = {
		// Long options (we'll do the rest after this struct).
		// We can't use these letters: MGmgby
		{"bi", required_argument, 0, 'a'},
		{"bcm", required_argument, 0, 'c'},
		{"bcx", required_argument, 0, 'd'},
		{"bvm", required_argument, 0, 'e'},
		{"bvx", required_argument, 0, 'f'},
		{"brf", required_argument, 0, 'g'},
		{"ys", required_argument, 0, 'h'},
		{"yps", required_argument, 0, 'i'},
		{"yna", no_argument, 0, 'j'},
		{"yp", required_argument, 0, 'k'},
		{"yv", required_argument, 0, 'l'},
		{"yc", required_argument, 0, 'n'},
		{0, 0, 0, 0}
	};

	int c, option_index = 0;

	while ((c = getopt_long_only(argc, argv, "byMGm:g:",
				long_options, &option_index)) != -1) {
		if (optarg)
			ext = optarg;

		switch(c) {
			case 0:
				// We go here if it's one of the long options.
				switch(*long_options[option_index].flag) {
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
					case 'i': // -yps  picsize
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
					case '?':
						success = false;
						break;
				}
				break;
			case 'y': // do Yee
				run_yee = true;
				++run_how_many;
				break;
			case 'b': // do Bayesian regret
				run_breg = true;
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
			case 'm':
				 constrain_methods = true;
				 method_constraint_fn = ext;
				 break;
			case 'g':
				 constrain_generators = true;
				 generator_constraint_fn = ext;
				 break;
			default:
				cout << option_index << endl;
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
		print_usage_info("foo");
		return(-1);
	}

	// Actions ahoy!
	// TODO: Let user specify truncation. Do so when truncation is actually
	// consistent with the logic of the generators instead of just being
	// something that's slapped on.

	list<election_method *> methods = get_singlewinner_methods(false);
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

	if (list_gen) {
		list_names(generators);
		return(0);
	}

	if (list_methods) {
		list_names(methods); // Aint templating grand?
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
		if (generators.empty()) {
			cerr << argv[0] << ": Found no eligible method names in"
				<< " " << generator_constraint_fn << endl;
			return(-1);
		}

		if (*generators.begin() == NULL)
			return(-1); // ditto.

		// If we get here, we have constrained, so se the default breg.
		// Dealloc the one we had first, though.
		delete *(default_br_generators.begin());

		default_br_generators = generators;

		cout << "Generator constraint: loaded " << generators.size()
			<< " generators." << endl;
	}

	// Run_yee and Run_breg goes here.
	// You need to prepare all the right stuff and then call either yee
	// or breg. After it's done, just use the same loop since they're both
	// instances of the ABC, mode.

	yee yee_mode;
	bayesian_regret br_mode;
	mode * mode_running;

	rng randomizer(get_abs_time()); // or 10 or somesuch. TODO: User spec.

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
	}

	string progress;
        double start_time = get_abs_time(), cur_checkpoint = start_time;
        while ( (progress = mode_running->do_round(true, false, randomizer)) 
			!= "") {
		cout << progress << endl;
		if (get_abs_time() - cur_checkpoint > 2) {
			double time_per_round = (get_abs_time()-start_time)/
				(double)mode_running->get_current_round();

			cout << "\t [Has run for " << get_abs_time()-start_time
				<< "s. ETA: " << time_per_round * (
						mode_running->get_max_rounds()-
						mode_running->
						get_current_round()) << "s. ]" 
				<< endl;
			cur_checkpoint = get_abs_time();
		}

		if (run_breg && mode_running->get_current_round() % 
				breg_report_freq == (breg_report_freq-1)) {
			vector<string> report = mode_running->provide_status();
			copy(report.begin(), report.end(), ostream_iterator<
					string>(cout, "\n"));
			cout << endl;
		}
	}
}
