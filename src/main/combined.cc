// All in one! The idea here is to build every possible simple method
// combination we can think of, then either dump it all so the user can specify
// which methods interest him, or intersect it with such a list winnowed down
// by the user. A more sophisticated version might try to parse the actual names
// so that we don't have to init every possible combination of methods
// beforehand. (BLUESKY do that, which would thus permit things like LE/LE/LE/
// (Smith,Plurality).)

#include <values.h>
#include <errno.h>
#include <ctype.h>

#include <getopt.h>

#include <stdexcept>
#include <iostream>
#include <fstream>
#include <list>

// General includes - tools and ballot structures.

#include "../tools/ballot_tools.h"
#include "../ballots.h"
#include "../tools/tools.h"

// Generators go here.

#include "../generator/all.h"

// Single-winner methods, including metamethods.
#include "../singlewinner/all.h"
#include "../singlewinner/get_methods.h"

// Interpreters for the interpreter mode
#include "../interpreter/all.h"

// Stats
#include "../stats/coordinate_gen.h"
#include "../stats/quasirandom/r_sequence.h"

// More later. POC for now. Perhaps also do something with the fact that C++
// doesn't have garbage collection -- i.e. clean up after ourselves.

// Headers for the Bayesian regret mode:
#include "../random/random.h"
#include "../stats/stats.h"

#include "../modes/breg.h"
#include "../modes/yee.h"

// Headers for the Yee mode:
#include "../images/color/color.h"

// Barycentric characterization mode
#include "../modes/barycentric.h"

// Interpreter itself.
#include "../modes/interpret.h"

// This should really be put into a file associated with all.h
std::vector<std::shared_ptr<pure_ballot_generator> > get_all_generators(
	bool compress, bool truncate) {

	std::vector<std::shared_ptr<pure_ballot_generator> > generators;

	generators.push_back(
		std::make_shared<dirichlet>(compress, truncate));
	generators.push_back(
		std::make_shared<gaussian_generator>(compress, truncate));
	generators.push_back(
		std::make_shared<impartial>(compress, truncate));
	generators.push_back(
		std::make_shared<dirichlet>(compress, truncate));
	generators.push_back(
		std::make_shared<iac>(compress, false)); // truncation not supported
	generators.push_back(
		std::make_shared<uniform_generator>(compress, truncate));

	return generators;
}

bayesian_regret setup_regret(
	std::vector<std::shared_ptr<election_method> > & methods,
	std::vector<std::shared_ptr<pure_ballot_generator> > & generators,
	int maxiters, int min_candidates, int max_candidates,
	int min_voters, int max_voters, uint64_t rng_seed) {

	// Do something with Bayesian regret here. DONE: Move over
	// to modes.

	bayesian_regret br(maxiters, min_candidates, max_candidates,
		min_voters, max_voters, false, MS_INTRAROUND,
		generators, methods);

	br.set_coordinate_gen(PURPOSE_MULTIPURPOSE,
		std::make_shared<rng>(rng_seed));

	// TODO: Throw the exception inside the bayesian regret code instead.
	if (!br.init()) {
		throw std::runtime_error("Bayesian Regret: could not initialize!");
	}

	return (br);
}

// To do Yee maps we need two coordinate sources: one that's random
// that decides the position of the candidates, and another that is used
// to sample the Gaussian (normal distribution) around the center as
// part of the Yee map. The first should be random, but the second
// doesn't need to be. Thus the candidate_coords_rng here is set to
// be an rng, not any coordinate generator. The other source will be used
// when calling do_round.

// TODO: Determine what uniform is used for, compare to gaussian.
// These should not be demanding particular types of generator,
// they should be requesting them for a certain purpose, and the
// caller should be able to substitute whatever it wants.

// I should refactor to separate out the distributions from the
// *ballot generators*. This would make spatial generators easy and
// not need hacks like getting candidate positions by peering into
// candscores.

yee setup_yee(
	std::vector<std::shared_ptr<election_method> > & methods,
	int num_voters, int num_cands,
	bool do_use_autopilot, std::string case_prefix, int picture_size,
	double sigma, spatial_generator & gaussian, uniform_generator & uniform,
	uint64_t rng_seed, bool quasi_monte_carlo) {

	yee to_output;

	if (!to_output.set_params(num_voters, num_cands, do_use_autopilot,
			case_prefix, picture_size, sigma)) {
		throw std::runtime_error("Yee diagram: Could not set parameters!");
	}

	std::shared_ptr<rng> rng_ptr = std::make_shared<rng>(rng_seed);

	to_output.set_coordinate_gen(PURPOSE_CANDIDATE_DATA, rng_ptr);

	if (quasi_monte_carlo) {
		to_output.set_coordinate_gen(PURPOSE_BALLOT_GENERATOR,
			std::make_shared<r_sequence>(2));
	} else {
		to_output.set_coordinate_gen(PURPOSE_BALLOT_GENERATOR,
			rng_ptr);
	}

	to_output.set_voter_pdf(&gaussian);
	to_output.set_candidate_pdf(&uniform);

	to_output.add_methods(methods.begin(), methods.end());

	if (!to_output.init()) {
		throw std::runtime_error("Yee diagram: Could not initialize!");
	}

	return (to_output);
}

barycentric setup_bary(
	std::vector<std::shared_ptr<election_method> > & methods) {

	barycentric to_output;

	to_output.add_methods(methods.begin(), methods.end());

	if (!to_output.init()) {
		throw std::runtime_error("Barycentric: Could not initialize!");
	}

	return (to_output);
}

std::pair<bool, interpreter_mode> setup_interpreter(
	std::vector<std::shared_ptr<election_method> > & methods,
	std::vector<std::shared_ptr<interpreter> > & interpreters,
	std::vector<std::string> & unparsed) {

	interpreter_mode toRet(interpreters, methods, unparsed);

	bool inited = toRet.init();

	if (!inited) {
		std::cerr << "Interpreter mode error: Cannot parse ballot data!"
			<< std::endl;
		return (std::pair<bool, interpreter_mode>(false, toRet));
	}

	return (std::pair<bool, interpreter_mode>(true, toRet));
}

std::pair<bool, interpreter_mode> setup_interpreter_from_file(
	std::vector<std::shared_ptr<election_method> > & methods,
	std::vector<std::shared_ptr<interpreter> > & interpreters,
	std::string file_name) {

	std::ifstream inf(file_name.c_str());

	if (!inf) {
		std::cerr << "Interpreter mode error: Could not open " <<
			file_name << " for reading!" << std::endl;
		return (std::pair<bool, interpreter_mode>(false, interpreter_mode()));
	}

	// Slurp the contents. Note, will crash the program if you feed it
	// /dev/zero or somesuch.

	std::vector<std::string> unparsed = slurp_file(inf, false);
	inf.close();

	return (setup_interpreter(methods, interpreters, unparsed));
}

/// --- ///

template<typename T> void list_names(const T & container) {

	for (typename T::const_iterator pos = container.begin(); pos !=
		container.end(); ++pos) {
		std::cout << (*pos)->name() << std::endl;
	}
}

// Get only those that appear on the list.

std::string ignore_spaces_np(const std::string & a) {

	std::string toRet;

	for (size_t counter = 0; counter < a.size(); ++counter) {
		if (isprint(a[counter]) && a[counter] != ' ') {
			toRet.push_back(a[counter]);
		}
	}

	return (toRet);
}

template<typename Q> std::vector<std::shared_ptr<Q> >
get_name_intersection(
	const std::vector<std::shared_ptr<Q> > & container,
	const std::vector<std::string> accepted) {

	// We do this by first building a map of Q with Q's name as index.
	// Then we go through the accepted list and dump those that we find.

	std::map<std::string, std::shared_ptr<Q> > codebook;

	for (std::shared_ptr<Q> ptr: container) {
		codebook[ignore_spaces_np(ptr->name())] = ptr;
	}

	std::vector<std::shared_ptr<Q> > to_return;

	for (std::vector<std::string>::const_iterator spos = accepted.begin();
		spos != accepted.end(); ++spos) {

		auto search = codebook.find(ignore_spaces_np(*spos));

		if (search != codebook.end()) {
			to_return.push_back(search->second);
		} else {
			std::cout << "Warning: could not find " << *spos << std::endl;
		}
	}

	return to_return;
}

template<typename Q> std::vector<Q> intersect_by_file(
	const std::vector<Q> & container, std::string filename) {

	// Returns a container of NULL if we can't open.

	std::ifstream infile(filename.c_str());

	if (!infile) {
		std::cerr << "Cannot open " << filename << " for reading!" << std::endl;
		return (std::vector<Q>(1, NULL));
	}

	std::vector<std::string> accepted = slurp_file(infile, true);
	infile.close();

	return get_name_intersection(container, accepted);
}

// ETA calculation and reasonable intervals (I won't call them CIs because
// the distribution can be arbitrarily skewed and thus the estimates can be
// arbitrarily wrong).

// This is *very* quick and dirty. For one, it uses standard random: it should
// use a secondary RNG seeded from an entropy source. Then I should say something
// about how the seed is not used for the ETA calculation etc... really, I should
// have multiple streams and dedicate one of them to this kind of "irrelevant"
// random number consumption.

double get_mean(const std::vector<double> & vec) {
	return std::accumulate(vec.begin(), vec.end(), 0.0) / (double)vec.size();
}

std::vector<double> bootstrap_means(const std::vector<double> & y,
	size_t bootstrap_samples) {

	std::vector<double> means;

	for (size_t i = 0; i < bootstrap_samples; ++i) {
		std::vector<double> bootstrap;

		for (size_t j = 0; j < y.size(); ++j) {
			bootstrap.push_back(y[random() % y.size()]);
		}

		means.push_back(get_mean(bootstrap));
	}

	// Sort the means because we're going to use them for interval
	// estimation.

	std::sort(means.begin(), means.end());

	return means;
}

struct time_estimate {
	double start_time, now;
	double fast_completion_time;
	double mean_completion_time;
	double slow_completion_time;
	double fast_eta, mean_eta, slow_eta;
};

time_estimate get_time_estimate(
	const std::vector<double> & time_elapsed_per_round,
	size_t cur_round, size_t total_rounds, double start_time,
	double time_now) {

	// TODO: Modify
	size_t bootstrap_samples = 500;

	// TODO: Should also do multiple testing correction here so that if
	// the ETA schedule is shown 1000 times, all the times as a whole
	// should have a 95% c.i. equivalent. (Shouldn't we?)
	double ci = 0.95;

	std::vector<double> means = bootstrap_means(time_elapsed_per_round,
			bootstrap_samples);

	// The returned means will be of time elapsed per instance, not
	// the total time expected until completion, so we need to do
	// a bit of juggling to translate.

	// It might also be better to project from now, i.e.
	// now + (total_rounds - current_instance) * ...
	// Doing so will ensure that the contribution of the prediction to
	// the total time shrinks to zero as we near the end. Fix later.

	time_estimate out;
	out.start_time = start_time;
	out.now = time_now;
	out.fast_completion_time = time_now +
		(total_rounds - cur_round) * means[means.size() * (1-ci)];
	out.slow_completion_time = time_now +
		(total_rounds - cur_round) * means[means.size() * ci];
	out.mean_completion_time = time_now +
		(total_rounds - cur_round) * get_mean(time_elapsed_per_round);

	out.fast_eta = out.fast_completion_time - time_now;
	out.slow_eta = out.slow_completion_time - time_now;
	out.mean_eta = out.mean_completion_time - time_now;

	return out;
}

void print_time_estimate(
	const std::vector<double> & time_elapsed_per_round,
	size_t cur_round, size_t total_rounds, double start_time,
	double time_now) {

	time_estimate this_estimate = get_time_estimate(time_elapsed_per_round,
			cur_round, total_rounds, start_time, time_now);

	std::cout << "\tEstimated time until completion: " <<
		this_estimate.mean_eta << "s (" << this_estimate.fast_eta
		<< "s - " << this_estimate.slow_eta << "s)" << std::endl;
	std::cout << "\tFinish times: " << this_estimate.mean_completion_time -
		start_time << "s ("
		<< this_estimate.fast_completion_time - start_time << " - " <<
		this_estimate.slow_completion_time - start_time << ")" << std::endl;
	std::cout << "\tNow: " << time_now - start_time << std::endl;
}

void print_usage_info(std::string program_name) {

	std::cout << "Quadelect, election method analysis tool." << std::endl <<
		std::endl;

	std::cout << "Usage: " << program_name << " [OPTIONS]... " << std::endl;
	std::cout << std::endl;

	std::cout << "General enumeration options:" << std::endl;
	std::cout << "\t-G\t\t List available ballot generators." << std::endl;
	// todo, implement!
	std::cout << "\t-I\t\t List available interpreters." << std::endl;
	std::cout << "\t-M\t\t List available election methods." << std::endl;
	std::cout << std::endl;
	std::cout << "Random number generator options:" << std::endl;
	std::cout <<
		"\t-r [seed]\t Set the random number generator seed to [seed]."
		<< std::endl << std::endl;
	std::cout << "Constraint options: " << std::endl;
	std::cout <<
		"\t-e\t\tEnable experimental methods. These are not intended for"
		<<
		"\n\t\t\tordinary use!" << std::endl;
	std::cout << "\t-g [file]\tUse the generators listed in [file]. The " <<
		"program will\n\t\t\tabort if that implies no generators " <<
		"are to be used." << std::endl;
	std::cout << "\t-ic [file]\tOnly use the interpreters listed in [file]."<<
		" The\n\t\t\tprogram will abort if that implies no interpreters"
		<<" are\n\t\t\tto be used." << std::endl;
	std::cout <<
		"\t-m [file]\tOnly use the election methods listed in [file]."<<
		" The\n\t\t\tprogram will abort if that implies no methods " <<
		"are to be\n\t\t\tused." << std::endl;
	std::cout << std::endl;
	std::cout << "Bayesian regret calculation options:" << std::endl;
	std::cout << "\t-b\t\tEnable the Bayesian Regret calculation mode." <<
		std::endl;
	std::cout <<
		"\t-bi [maxiters]\tDo a maximum of [maxiters] rounds.\n\t\t\t"<<
		"Default is 20000." << std::endl;
	std::cout << "\t-bcm [cands]\tUse a minimum of [cands] candidates when"
		<< " performing a\n\t\t\tBayesian regret round. Default is 3."
		<< std::endl;
	std::cout << "\t-bcx [cands]\tUse a maximum of [cands] candidates when"
		<< " performing a\n\t\t\tBayesian regret round. Default is 20."
		<< std::endl;
	std::cout << "\t-bvm [voters]\tUse a minimum of [voters] voters when"
		<< " performing a\n\t\t\tBayesian regret round. Default is 4."
		<< std::endl;
	std::cout << "\t-bvx [voters]\tUse a maximum of [voters] voters when"
		<< " performing a\n\t\t\tBayesian regret round. Default is 200."
		<< std::endl;
	std::cout << "\t-brf [rounds]\tPrint Bayesian regret statistics every " <<
		"[rounds] rounds.\n\t\t\tDefault is 100." << std::endl;
	std::cout << std::endl;
	std::cout << "Interpreter (ballot counting) options: " << std::endl;
	std::cout << "\t-i\t\tEnable the interpreter/ballot counting mode." <<
		std::endl;
	std::cout <<
		"\t-if [file]\tUse [file] as the input file to interpret. The";
	std::cout << "\n\t\t\tdefault is \"interpret.txt\"." << std::endl;
	// todo, more here!
	std::cout << std::endl;
	std::cout << "Yee diagram calculation options:" << std::endl;
	std::cout << "\t-y\t\tEnable the Yee diagram calculation/rendering mode."
		<< std::endl;
	std::cout << "\t-ys [sigma]\tSet the standard deviation of the voter " <<
		"Gaussian\n\t\t\tto [sigma]. Default is 0.3" << std::endl;
	std::cout << "\t-yps [pixels]\tRender a [pixels]*[pixels] Yee diagram." <<
		"\n\t\t\tDefault is 240." << std::endl;
	std::cout << "\t-yna\t\tDon't use progressive sampling (\"autopilot\"). "
		<<
		"Disabling\n\t\t\tthis will turn the rendering slower." << std::endl;
	std::cout << "\t-yp [prefix]\tPrefix the rendered picture names by " <<
		"[prefix].\n\t\t\tDefault is \"default\"" << std::endl;
	std::cout << "\t-yv [voters]\tSample a maximum of [voters] voters per " <<
		"pixel.\n\t\t\tDefault is 1000." << std::endl;
	std::cout << "\t-yc [cands]\tPlace [cands] random candidates on the Yee"<<
		" map.\n\t\t\tDefault is 4." << std::endl;
	std::cout << "\t-yq\t\tUse Quasi-Monte Carlo. This generally gives more"<<
		"\n\t\t\taccurate results, but requires autopilot to be"<<
		"\n\t\t\tdisabled and may have side effects on methods"<<
		"\n\t\t\tthat break ties randomly. Default is no.\n" << std::endl;
	std::cout << std::endl;
	std::cout << "Barycentric characterization options:" << std::endl;
	std::cout << "\t-c\t\tEnable voter method barycentric visualization." <<
		std::endl;
}

int main(int argc, char * * argv) {

	// Set parameter defaults.

	double yee_sigma = 0.3;
	int yee_size = 240, yee_voters = 1000, yee_candidates = 4;
	bool yee_autopilot = true, yee_quasi_mc = false;
	std::string yee_prefix = "default";

	int breg_rounds = 20000, breg_min_cands = 3, breg_max_cands = 20,
		breg_min_voters = 4, breg_max_voters = 200, breg_report_freq = 100;

	bool run_yee = false, run_breg = false, run_int = false, run_bary = false,
		 list_methods = false, list_gen = false, list_int = false;
	int run_how_many = 0;

	bool constrain_methods = false, constrain_generators = false,
		 constrain_ints = false;

	// TODO: Silently add experimental methods but don't show them in the
	// listing provided to the user so that they can still be forced if
	// necessary.
	bool include_experimental = false;

	std::string method_constraint_fn, generator_constraint_fn,
		int_constraint_fn;

	std::string int_source_file = "interpret.txt";

	rng randomizer(get_abs_time());
	uint64_t seed = randomizer.next_long();

	bool success = true;

	std::string ext;

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
		{"yq", no_argument, 0, 's'},
		{0, 0, 0, 0}
	};

	int c, option_index = 0, opt_flag;

	while ((c = getopt_long_only(argc, argv, "ebycMGIim:g:r:s",
					long_options, &option_index)) != -1) {

		if (option_index != 0) {
			c = 0;    // Access the long options!
		}

		if (optarg) {
			ext = optarg;
		}

		switch (c) {
			case 0:
				opt_flag = long_options[option_index].val;
				option_index = 0; // Don't confuse later params.

				// We go here if it's one of the long options.
				switch (opt_flag) {
					case 'a': // -bi
						breg_rounds = str_toi(ext);
						if (breg_rounds <= 0) {
							std::cerr << "You must specify at least one round."
								<< std::endl;
							return -1;
						}
						break;
					case 'c': // -bcm  mincand
						breg_min_cands = str_toi(ext);
						if (breg_min_cands <= 1) {
							std::cerr << "Must have at least two candidates."
								<< std::endl;
							return -1;
						}
						break;
					case 'd': // -bcx  maxcand
						breg_max_cands = str_toi(ext);
						if (breg_max_cands <= 1) {
							std::cerr << "Must have at least two candidates."
								<< std::endl;
							return -1;
						}
						break;
					case 'e': // -bvm  minvoter
						breg_min_voters = str_toi(ext);
						if (breg_min_voters < 1) {
							std::cerr << "Must have at least one voter."
								<< std::endl;
							return -1;
						}
						break;
					case 'f': // -bvx  maxvoter
						breg_max_voters = str_toi(ext);
						if (breg_max_voters < 1) {
							std::cerr << "Must have at least one voter."
								<< std::endl;
							return -1;
						}
						break;
					case 'g': // -brf  report_freq
						breg_report_freq = str_toi(ext);
						assert(breg_report_freq > 0);
						break;
					case 'h': // -ys   sigma
						yee_sigma = str_tod(ext);
						if (yee_sigma < 0) {
							std::cerr << "Yee diagram: standard deviation must "
								"be nonnegative." << std::endl;
							return -1;
						}
						break;
					case 'o': // -yps  picsize
						yee_size = str_toi(ext);
						if (yee_size < 0) {
							std::cerr << "Yee diagram: picture output size must "
								"be nonnegative." << std::endl;
							return -1;
						}
						break;
					case 'j': // -yna  autopilot
						yee_autopilot = false;
						break;
					case 'k': // -yp   prefix
						yee_prefix = ext;
						break;
					case 'l': // -yv   voters
						yee_voters = str_toi(ext);
						if (yee_voters < 1) {
							std::cerr << "Yee diagram: must have at "
								"least one voter." << std::endl;
							return -1;
						}
						break;
					case 'n': // -yc   candidates
						yee_candidates = str_toi(ext);
						if (yee_candidates < 1) {
							std::cerr << "Yee diagram: must have at "
								"least two candidates." << std::endl;
							return -1;
						}
						break;
					case 's': // -yq	Yee: enable quasi-gaussian
						yee_quasi_mc = true;
						yee_autopilot = false; // Autopilot interferes
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
			case 'c': // do barycentric characterization
				run_bary = true;
				++run_how_many;
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
				if (option_index != 0) {
					option_index = 0;
				} else {
					/*std::cout << "I pity the foo. " << (char)c
						<< std::endl;*/
					success = false;
				}
		}
	}

	if (run_how_many > 1) {
		std::cerr << argv[0] << ": more than one action specified. Please "
			<< "be more specific." << std::endl;
		return (-1);
	}

	// If he made a booboo with the parameters or if he didn't specify
	// anything to do, print usage information and get outta here.
	if (!success || run_how_many == 0) {
		print_usage_info(argv[0]);
		return (-1);
	}

	// Actions ahoy!

	// While I'm porting things over to use smart pointers, this hack
	// will have to do: it keeps a smart pointer list so that everything
	// gets properly deallocated once the program exits, but lets other
	// functions use raw pointers. Apparently "proper" code is supposed
	// to make a distinction between smart and raw pointers -- use the
	// former if the function can extend something's lifetime, otherwise
	// the latter, but that sounds rather too ugly so I'm just choosing
	// something.

	std::vector<std::shared_ptr<election_method> > methods =
		get_singlewinner_methods(false, include_experimental);

	// TODO: Let user specify truncation. Do so when truncation is actually
	// consistent with the logic of the generators instead of just being
	// something that's slapped on.

	std::vector<std::shared_ptr<pure_ballot_generator> > generators =
		get_all_generators(true, false);

	std::vector<std::shared_ptr<pure_ballot_generator> > default_br_generators;
	default_br_generators.push_back(
		std::make_shared<uniform_generator>(true, false));

	// Interpreters for the interpreter mode
	std::vector<std::shared_ptr<interpreter> > interpreters =
		get_all_interpreters();

	if (list_gen) {
		list_names(generators);
		return (0);
	}

	if (list_methods) {
		list_names(methods); // Aint templating grand?
		return (0);
	}

	if (list_int) {
		list_names(interpreters);
		return (0);
	}

	// Constraining goes here.

	if (constrain_methods) {
		methods = intersect_by_file(methods, method_constraint_fn);
		if (methods.empty()) {
			std::cerr << argv[0] << ": Found no eligible method names in"
				<< " " << method_constraint_fn << std::endl;
			return (-1);
		}
		if (*(methods.begin()) == NULL) {
			return (-1);    // couldn't open file.
		}

		std::cout << "Method constraint: loaded " << methods.size()
			<< " methods." << std::endl;
	}

	if (constrain_generators) {
		generators = intersect_by_file(generators,
				generator_constraint_fn);
		if (generators.empty() || *generators.begin() == NULL) {
			std::cerr << argv[0] << ": Found no eligible generator names in"
				<< " " << generator_constraint_fn << std::endl;
			return (-1);
		}

		// If we get here, we have constrained, so use the default breg.
		default_br_generators = generators;

		std::cout << "Generator constraint: loaded " << generators.size()
			<< " generators." << std::endl;
	}

	if (constrain_ints) {
		interpreters = intersect_by_file(interpreters,
				int_constraint_fn);
		if (interpreters.empty() || *interpreters.begin() == NULL) {
			std::cerr << argv[0] << ": Found no eligible interpreter " <<
				"names in " << int_constraint_fn << std::endl;
			return (-1);
		}

		std::cout << "Interpreter constraint: loaded " << interpreters.size()
			<< " interpreters." << std::endl;
	}

	std::cout << "Random number generator: using seed " << seed << std::endl;
	randomizer.s_rand(seed);

	// Run_yee, Run_breg, run_bary goes here.
	// You need to prepare all the right stuff and then call either yee
	// or breg. After it's done, just use the same loop since they're both
	// instances of the ABC, mode.

	yee yee_mode;
	bayesian_regret br_mode;
	barycentric bary_mode;
	std::pair<bool, interpreter_mode> int_mode;
	mode * mode_running;

	uniform_generator uniform(true, false);
	gaussian_generator gaussian(true, false);

	if (run_yee) {
		std::cout << "Setting up Yee..." << std::endl;
		std::cout << "\t\t- sigma: " << yee_sigma << std::endl;
		std::cout << "\t\t- picture size: " << yee_size << std::endl;
		std::cout << "\t\t- integration mode: ";
		if (yee_quasi_mc) {
			std::cout << "Quasi-Monte Carlo\n";
		} else {
			std::cout << "Monte Carlo\n";
		}
		std::cout << "\t\t- max number of voters: " << yee_voters << std::endl;
		std::cout << "\t\t- number of candidates: " << yee_candidates << std::endl;
		std::cout << "\t\t- autopilot: ";
		if (yee_autopilot) {
			std::cout << "yes" << std::endl;
		} else	{
			std::cout << "no" << std::endl;
		}
		std::cout << "\t\t- picture prefix: " << yee_prefix << std::endl;

		if (methods.size() > 10) {
			std::cout << "WARNING: You have selected more than 10 " <<
				"methods at once. This may take a *very* " <<
				"long time." << std::endl;
		}

		yee_mode = setup_yee(methods, yee_voters, yee_candidates,
				yee_autopilot, yee_prefix, yee_size, yee_sigma,
				gaussian, uniform, randomizer.get_initial_seed(),
				yee_quasi_mc);

		yee_mode.print_candidate_positions();

		mode_running = &yee_mode;

	}

	if (run_breg) {
		std::cout << "Setting up Bayesian Regret..." << std::endl;
		std::cout << "\t\t- rounds: " << breg_rounds << std::endl;
		std::cout << "\t\t- minimum number of candidates: " << breg_min_cands
			<< std::endl;
		std::cout << "\t\t- maximum number of candidates: " << breg_max_cands
			<< std::endl;
		std::cout << "\t\t- minimum number of voters: " << breg_min_voters
			<< std::endl;
		std::cout << "\t\t- maximum number of voters: " << breg_max_voters
			<< std::endl;
		std::cout << "\t\t- report frequency: " << breg_report_freq << std::endl;

		br_mode = setup_regret(methods, generators,
				breg_rounds, breg_min_cands, breg_max_cands,
				breg_min_voters, breg_max_voters,
				randomizer.get_initial_seed());

		mode_running = &br_mode;
	}

	if (run_bary) {
		std::cout << "Setting up barycentric visualization" << std::endl;

		bary_mode = setup_bary(methods);

		mode_running = &bary_mode;
	}

	if (run_int) {
		std::cout << "Setting up ballot interpretation..." << std::endl;
		std::cout << "\t\t- input file: " << int_source_file << std::endl;
		std::cout << "\t\t- number of interpreters: " << interpreters.size()
			<< std::endl;
		std::cout << "\t\t- number of methods: " << methods.size() << std::endl;

		int_mode = setup_interpreter_from_file(methods, interpreters,
				int_source_file);

		if (!int_mode.first) {
			return (-1);    // Something wrong, outta here.
		}

		mode_running = &int_mode.second;
	}

	std::string progress;
	double start_time = get_abs_time(), cur_checkpoint = start_time;

	std::vector<double> time_elapsed_per_round;

	while ((progress = mode_running->do_round(true)) != "") {
		std::cout << progress << std::endl;

		double this_instance = (get_abs_time() - cur_checkpoint);
		time_elapsed_per_round.push_back(this_instance);

		print_time_estimate(time_elapsed_per_round,
			mode_running->get_current_round(),
			mode_running->get_max_rounds(), start_time,
			get_abs_time());

		cur_checkpoint = get_abs_time();

		bool should_display_stats = false;
		// If we're at the last round or if at the breg_report_freq and
		// Bayesian regret, report.

		if (mode_running->get_current_round() ==
			mode_running->get_max_rounds()) {
			should_display_stats = true;
		} else	should_display_stats = run_breg &&
				mode_running->get_current_round() %
				breg_report_freq == (breg_report_freq-1);

		if (should_display_stats) {
			std::vector<std::string> report = mode_running->provide_status();
			copy(report.begin(), report.end(), std::ostream_iterator<
				std::string>(std::cout, "\n"));
			std::cout << std::endl;
		}
	}
}
