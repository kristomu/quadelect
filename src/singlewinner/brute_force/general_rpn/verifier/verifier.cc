#include "../composition/groups/test_generator_groups.h"
#include "../composition/groups/test_generator_group.h"

#include "../composition/test_results.h"

#include "../../../../linear_model/constraints/relative_criterion_producer.h"
#include "../../../../linear_model/constraints/relative_criteria/mono-raise.h"
#include "../../../../linear_model/constraints/relative_criteria/mono-add-top.h"

#include "../../../../config/general_rpn.h"

#include "../../../../tools/time_tools.h"

#include <iostream>
#include <iterator>
#include <memory>
#include <queue>

#include "../isda.cc"

// This program takes files generated by polytope_compositor to determine
// what combinations of algorithms for the different scenarios produce
// viable election methods (i.e. ones that pass the tests tested by
// polytope_compositor).

// The program does the actual checking by a backtracking recursive
// algorithm that consists of alternating between assigning algorithms to
// unassigned scenarios and checking if they pass tests.

// However, that still leaves the question of in what order to investigate
// the different test groups. We would want to put the most discriminating
// tests first, so that the backtracking aborts the recursive tests as early
// as possible (thus avoiding the combinatorial explosion problem).

// Previously, I determined the order by using a topological sort, but that
// led to bad assignments (where some very discriminating tests were pushed
// to the back), so instead I now use a local search: try every test group
// alone and check how many tests pass it after 2 seconds, then pick the one
// that pass the fewest tests and repeat with the next test group.

///////////////////////////////////////////////////////////////////////////

// Once we know what order to investigate, we can do a recursive
// enumeration. We set up a vector listing what algorithm to try for
// what scenario (-1 if we haven't decided yet), and go through each data
// file corresponding to each test generator group, in order.

// When we enter into a new test generator group, we go through the four
// scenarios (A, B, A', B') and check if we've assigned algorithms to each.
// If we haven't, we run a for loop to try every possible algorithm for the
// unused positions. At the end, all four scenario positions are populated
// and we can test if this configuration passes the tests according to the
// data file produced by compositor.

class test_and_result {
	public:
		test_generator_group test_group;
		test_results group_results;

		test_and_result(const test_generator_group & group_in,
			const test_results & results_in) : test_group(group_in),
			group_results(results_in) {}
};

class backtracker {
	private:
		// returns fraction covered.
		double get_progress() const;
		void print_progress() const;

		std::map<copeland_scenario, int> algorithm_used_idx_for_scenario;
		std::vector<std::vector<int> > algorithm_used_idx_for_test;
		std::vector<int> algorithm_used;
		std::vector<size_t> numcands_per_scenario_idx;

		void init_algorithms_used();

		void try_algorithms(size_t test_group_idx,
			test_election current_election_setting);

		std::vector<size_t> iteration_count;
		std::vector<test_and_result> tests_and_results;

		std::vector<std::vector<int> > algorithm_per_setting;
		std::vector<gen_custom_function> evaluators;
		size_t min_numcands, max_numcands;
		size_t num_groups;

		// for showing a progress report without wasting too much time
		// on time-elapsed calls.
		size_t global_counter, counter_threshold;
		time_pt last_shown_time, start_time;

		double progress_at_exit;

	public:
		// Should be made private once a few things have been improved/
		// refactored.
		std::vector<std::vector<algo_t> > prospective_algorithms;
		bool show_reports;

		// Abort after this time has elapsed, or -1 if no such limit
		// is desired.
		double time_limit = 0;

		// for calculating the progress
		// needs to be improved.
		std::vector<std::vector<int> > max_num_algorithms;

		void set_tests_and_results(
			const std::list<size_t> & order,
			const test_generator_groups & all_groups,
			const std::vector<test_results> & all_results);

		void set_algorithms(const std::vector<vector<algo_t> > & algos_in);

		double try_algorithms() {
			if (prospective_algorithms.empty()) {
				throw new std::runtime_error("No algorithms to check!");
			}

			start_time = get_now();
			progress_at_exit = 0;
			try_algorithms(0, TYPE_A);

			if (show_reports) {
				std::cout << "Final iteration counts." << std::endl;
				for (size_t i = 0; i < iteration_count.size(); ++i) {
					std::cout << "idx " << i << "\t" << iteration_count[i] << std::endl;
				}
			}

			// If progress at exit is not set, we must have gone through
			// the whole search space before exceeding our time budget, so
			// set progress to 1 (for 100% completion).
			if (progress_at_exit == 0) {
				progress_at_exit = 1;
			}

			return progress_at_exit;
		}

		backtracker(size_t min_numcands_in, size_t max_numcands_in) {
			min_numcands = min_numcands_in; // remove later
			max_numcands = max_numcands_in;
			counter_threshold = 2000;
			global_counter = 0;
			last_shown_time = get_now();
			progress_at_exit = 0;
			show_reports = true;
			time_limit = -1;

			for (size_t i = 0; i <= max_numcands; ++i) {
				evaluators.push_back(gen_custom_function(i));
			}
		}
};

double backtracker::get_progress() const {
	// This is basically a least significant digit first radix conversion.
	// While MSD is easier to write, it would also cause overflows.
	// idx is indexed backwards to avoid problems with going below zero
	// on an unsigned counter.

	// Fortunately for us, algorithm_used is indexed so that [0] changes
	// least often, [1] changes a bit more often, and so on, so we can
	// simply go in reverse order, skipping any -1 values.

	double progress = 0;

	for (size_t r_idx = 1; r_idx <= algorithm_used.size(); ++r_idx) {
		size_t idx = algorithm_used.size() - r_idx;

		if (algorithm_used[idx] == -1) { continue; }

		progress += algorithm_used[idx];
		progress /= (double)prospective_algorithms[
			numcands_per_scenario_idx[idx]].size();
	}

	return progress;
}

void backtracker::print_progress() const {

	double progress = get_progress();

	double seconds_run = to_seconds(start_time, last_shown_time);
	double eta_seconds = (1-progress)/progress * seconds_run;

	std::cerr << "Iteration counts." << std::endl;
	for (size_t i = 0; i < iteration_count.size(); ++i) {
		std::cerr << "idx " << i << "\t" << iteration_count[i] << std::endl;
	}

	std::cerr << "Progress: " << progress << "    ";
	std::cerr << "ETA: " << format_time(eta_seconds) << ".";
	std::cerr << "    \r" << std::flush;
}

// We want to assign an algorithm to each scenario, which then determines
// what voting method we have constructed out of the algorithms. Because
// indexing by scenario takes too long time when we're calling the
// try_algorithms function billions of times, instead the program uses a
// bit of an in-between move. It assigns each (group_idx, election_setting)
// pair to an index into an array. What that index is depends on what
// scenario corresponds to that particular group and that particular
// election setting. Then the try_algorithms function can simply check if
// some algorithm has already been set at algorithm_used[
// algorithm_used_idx_for_test[test_group_idx][current_election_setting]]
// without having to do a costly map lookup.
void backtracker::init_algorithms_used() {

	algorithm_used_idx_for_scenario.clear();

	algorithm_used_idx_for_test = std::vector<std::vector<int> >(
		tests_and_results.size(), std::vector<int>(NUM_REL_ELECTION_TYPES,
			0));

	max_num_algorithms = std::vector<std::vector<int> >(
		tests_and_results.size(), std::vector<int>(NUM_REL_ELECTION_TYPES,
			0));

	// Map scenarios to indices and (test_group, current_election) pairs
	// to the same indices through their scenarios.
	size_t num_distinct_scenarios = 0, test_group_idx;
	int current_election_setting;

	numcands_per_scenario_idx = std::vector<size_t>();

	for (test_group_idx = 0; test_group_idx < tests_and_results.size();
		++test_group_idx) {

		for (current_election_setting = 0; current_election_setting <
			NUM_REL_ELECTION_TYPES; ++current_election_setting) {

			copeland_scenario current_scenario = tests_and_results[
				test_group_idx].test_group.get_scenario(
				(test_election)current_election_setting);

			if (algorithm_used_idx_for_scenario.find(current_scenario) ==
				algorithm_used_idx_for_scenario.end()) {
				algorithm_used_idx_for_scenario[current_scenario] =
					num_distinct_scenarios;

				numcands_per_scenario_idx.push_back(
					current_scenario.get_numcands());

				num_distinct_scenarios++;
			}

			algorithm_used_idx_for_test[test_group_idx]
				[current_election_setting] =
				algorithm_used_idx_for_scenario[current_scenario];

			// Set the denominator for the progress indicator.
			// This could be done once and for all outside of the loop.
			// TODO: Do that.
			max_num_algorithms[test_group_idx][(int)current_election_setting] =
				prospective_algorithms[current_scenario.get_numcands()].size();
		}
	}

	algorithm_used = std::vector<int>(num_distinct_scenarios, -1);
}

void backtracker::try_algorithms(size_t test_group_idx,
	test_election current_election_setting) {

	if (test_group_idx == num_groups) {
		++iteration_count[test_group_idx];

		if (!show_reports) { return; }

		std::cout << "Reached the end." << std::endl;

		for (const auto & kv : algorithm_used_idx_for_scenario) {
			size_t numcands = kv.first.get_numcands();

			int algo_idx = algorithm_used[kv.second];
			algo_t algorithm = prospective_algorithms[numcands][algo_idx];
			evaluators[numcands].set_algorithm(algorithm);
			std::cout << "\t" << kv.first.to_string() << ": "
				<< algorithm << "\t" << evaluators[numcands].to_string()
				<< "\n";
		}
		std::cout << "Summary: ";
		for (const auto & kv : algorithm_used_idx_for_scenario) {
			size_t numcands = kv.first.get_numcands();

			int algo_idx = algorithm_used[kv.second];
			algo_t algorithm = prospective_algorithms[numcands][algo_idx];
			evaluators[numcands].set_algorithm(algorithm);

			std::cout << evaluators[numcands].to_string() << " ## ";
		}

		std::cout << std::endl;

		std::cout << "Iteration counts." << std::endl;
		for (size_t i = 0; i < iteration_count.size(); ++i) {
			std::cout << "idx " << i << "\t" << iteration_count[i] << std::endl;
		}
		return;
	}

	// First check if we've set an algorithm for the current test group.
	// If not, go through every possible algorithm.

	int current_scenario_idx = algorithm_used_idx_for_test[test_group_idx]
		[(int)current_election_setting];

	size_t i, numcands = numcands_per_scenario_idx[current_scenario_idx];

	if (algorithm_used[current_scenario_idx] == -1) {
		// Go through every possible algorithm. For each, recurse back with
		// the current position the same so we'll fall through next time.

		for (i = 0; i < prospective_algorithms[numcands].size(); ++i){
			algorithm_used[current_scenario_idx] = i;
			try_algorithms(test_group_idx, current_election_setting);
		}

		// Since we've looped through to ourselves, there's no need
		// to do anything but reset the scenario to undecided and return.
		algorithm_used[current_scenario_idx] = -1;
		return;
	}

	// If we got here, the algorithm to use for the current scenario has
	// already been defined. So set the algorithm_per_setting array to
	// this particular algorithm, as the testing function need is in that
	// particular format.

	// Increment the relevant iteration count.
	if (current_election_setting == TYPE_A) {
		++iteration_count[test_group_idx];
	}

	// Note that we need a different algorithm_per_setting array for each
	// test_group_idx. Otherwise recursions further in might scribble on
	// an algorithm_per_setting that a recursion further out needs to
	// preserve.

	algorithm_per_setting[test_group_idx][(int)current_election_setting] =
		algorithm_used[current_scenario_idx];

	// Show a progress report if the global counter is high enough and enough
	// time has elapsed. (1s hard-coded.)
	if (global_counter++ >= counter_threshold) {

		// Time limit check
		if (time_limit > 0 &&
			to_seconds(start_time, get_now()) >= time_limit) {

			// Record how far we managed to get before forced to exit,
			// if we haven't already recorded it.
			if (progress_at_exit == 0) {
				progress_at_exit = get_progress();
			}
			return;
		}

		global_counter = 0;
		if (show_reports && to_seconds(last_shown_time,get_now()) > 1) {

			last_shown_time = get_now();
			print_progress();
		}
	}

	// If we're at the last election setting, run a test, because we have
	// algorithms for every selection setting (A, B, A', B').
	if (current_election_setting == TYPE_B_PRIME) {
		bool pass = tests_and_results[test_group_idx].group_results.
			passes_tests(algorithm_per_setting[test_group_idx],
				tests_and_results[test_group_idx].test_group.get_no_harm(),
				tests_and_results[test_group_idx].test_group.get_no_help());

		// Abort early if no pass.
		if (!pass) { return; }
	}

	// Recurse either to the next group or to the next election setting.
	if (current_election_setting == TYPE_B_PRIME) {
		try_algorithms(test_group_idx+1, TYPE_A);
	} else {
		try_algorithms(test_group_idx, (test_election)
			((int)current_election_setting+1));
	}

	// Make debugging easier by cleaning up after ourselves.
	algorithm_per_setting[test_group_idx][(int)current_election_setting] = -1;
}

void backtracker::set_tests_and_results(
	const std::list<size_t> & order,
	const test_generator_groups & all_groups,
	const std::vector<test_results> & all_results) {

	tests_and_results.clear();

	for (size_t idx: order) {
		tests_and_results.push_back(
			test_and_result(all_groups.groups[idx], all_results[idx]));
	}

	algorithm_per_setting =
		std::vector<std::vector<int> >(tests_and_results.size(),
			std::vector<int>(NUM_REL_ELECTION_TYPES, -1));

	max_num_algorithms = std::vector<std::vector<int> >(
		tests_and_results.size(),
			std::vector<int>(NUM_REL_ELECTION_TYPES, -1));

	iteration_count =
		std::vector<size_t>(tests_and_results.size()+1, 0);

	// Init_algorithms_used also initializes the size counts that we
	// need for progress reports. These counts depend on the algorithms
	// having been set, so we can only call init_algorithms if both
	// tests_and_results and prospective_algorithms have been set.

	if (!prospective_algorithms.empty()) {
		init_algorithms_used();
	}

	num_groups = tests_and_results.size();
}

void backtracker::set_algorithms(
	const std::vector<vector<algo_t> > & algos_in) {

	prospective_algorithms = algos_in;

	// See above.
	if (!tests_and_results.empty()) {
		init_algorithms_used();
	}
}

/////////////////////////////////////////////////////////////////

// "Greater" sorts descending by score and ascending by group index,
// which is how the tiebreaking should work for aesthetic reasons.
class group_score_pair {
	public:
		double score;
		size_t group_idx;

		bool operator>(const group_score_pair & other) const {
			if (score != other.score) { return score < other.score; }
			return group_idx >= other.group_idx;
		}

		group_score_pair(double score_in, size_t idx_in) {
			group_idx = idx_in;
			score = score_in;
		}
};

// Local search subroutine: try the same search with randomized algorithm
// tables every time to get a better idea of the current group arrangement
// works on different areas of the search space.

double get_mean_progress(int tries, double time_limit,
	backtracker & tester) {

	// Without taking into account the time it takes to shuffle. I know.
	double time_limit_per = time_limit / (double)tries;
	double old_time_limit = tester.time_limit;
	tester.time_limit = time_limit_per;

	double progress_count = 0;

	for (int i = 0; i < tries; ++i) {
		// Violate the law of Demeter left and right.
		for (size_t j = 0; j < tester.prospective_algorithms.size(); ++j) {
			std::random_shuffle(tester.prospective_algorithms[j].begin(),
				tester.prospective_algorithms[j].end());
		}
		progress_count += tester.try_algorithms();
	}

	tester.time_limit = old_time_limit;
	return progress_count/(double)tries;
}

std::list<size_t> get_group_order(const test_generator_groups & groups,
	const std::vector<test_results> & all_results, backtracker & tester,
	const std::vector<std::vector<algo_t> > & functions_to_test,
	bool report) {

	std::priority_queue<group_score_pair, std::vector<group_score_pair >,
		std::greater<group_score_pair > > incoming_groups,
		outgoing_groups;

	std::list<size_t> output_order;

	bool old_reports = tester.show_reports;

	tester.show_reports = false;
	double time_limit = 0.1;

	// Dump every group into the incoming_groups priority queue.
	for (size_t i = 0; i < groups.groups.size(); ++i) {
		incoming_groups.push(group_score_pair(0, i));
	}

	while (!incoming_groups.empty()) {
		// Visit the incoming queue in order from the furthest progressing
		// to the least, as a proxy for how far it will progress this
		// time around.

		while (!incoming_groups.empty()) {
			std::list<size_t> tentative = output_order;
			tentative.push_back(incoming_groups.top().group_idx);

			tester.set_tests_and_results(tentative, groups, all_results);

			double progress = get_mean_progress(4, time_limit, tester);

			if (report) {
				std::cout << incoming_groups.top().group_idx << ": progress at"
					" exit was " << progress << "\n";
			}

			outgoing_groups.push(group_score_pair(progress,
					incoming_groups.top().group_idx));
			incoming_groups.pop();
		}
		// Insert the top as the next element of the output order.
		if (report) {
			std::cout << "Inserting recordholder " <<
				outgoing_groups.top().group_idx << " with progress " <<
				outgoing_groups.top().score << "." << std::endl;
		}

		output_order.push_back(outgoing_groups.top().group_idx);
		outgoing_groups.pop();

		// Add any indices that has progress level 1. If what we have
		// so far finishes within the time limit, then every subsequent
		// test will return progress before exit = 1, which means there's
		// no point in testing them more than once.
		while (!outgoing_groups.empty() &&
			outgoing_groups.top().score == 1) {

			output_order.push_back(outgoing_groups.top().group_idx);
			outgoing_groups.pop();
		}


		swap(incoming_groups, outgoing_groups);
	}

	tester.show_reports = old_reports;

	// get_mean_progress musses up the order of the algorithms, so
	// fix that by resetting the tester's algorithms.
	tester.set_algorithms(functions_to_test);

	return output_order;
}

void print_group_order(const std::list<size_t> & group_order) {
	std::cout << "group_order = [";
	// https://stackoverflow.com/questions/3496982
	for (std::list<size_t>::const_iterator iter = group_order.begin();
		iter != group_order.end(); iter++) {

		if (iter != group_order.begin()) {
			cout << ", ";
		}
		cout << *iter;
	}
	std::cout << "];\n";
}

int main(int argc, char ** argv) {
	size_t min_numcands = 3, max_numcands = 4;
	rng randomizer(RNG_ENTROPY);

	// Read algorithms from file.

	std::cout << "Reading files..." << std::endl;

	if (argc < 2) {
		std::cerr << "Usage: " << argv[0]
			<< " [general_rpn_tools config file] "
			<< std::endl;
		return(-1);
	}

	// Read configuration file.
	g_rpn_config settings;
	settings.load_from_file(argv[1]);

	std::vector<std::string> filename_cand_inputs = settings.source_files;

	size_t i;

	std::vector<std::vector<algo_t> > functions_to_test(max_numcands+1);

	for (i = min_numcands; i <= max_numcands; ++i) {
		std::ifstream sifter_file(filename_cand_inputs[i]);

		if (!sifter_file) {
			std::cerr << "Could not open " << filename_cand_inputs[i]
				<< std::endl;
		}

		get_first_token_on_lines(sifter_file, functions_to_test[i]);

		sifter_file.close();

		std::cout << "... done " << i << " candidates. (read "
			<< functions_to_test[i].size() <<  " functions)."
			<< std::endl;
	}

	// eof

	std::map<int, fixed_cand_equivalences> cand_equivs =
		get_cand_equivalences(max_numcands);

	std::vector<copeland_scenario> canonical_full_v;

	for (i = min_numcands; i <= max_numcands; ++i) {

		std::set<copeland_scenario> canonical_full = get_nonderived_scenarios(
			i, cand_equivs.find(i)->second);

		std::copy(canonical_full.begin(), canonical_full.end(),
			std::back_inserter(canonical_full_v));
	}

	// We need at least one relative constraint to make our groups, so
	// allocate them here.

	std::vector<std::shared_ptr<relative_criterion_const> >
		relative_constraints;

	// Add some relative constraints. (Kinda ugly, but what can you do.)
	relative_constraints = relative_criterion_producer().get_criteria(
		min_numcands, max_numcands, true, settings.desired_criteria);

	// Create all the groups
	// There seem to be some bugs where the same group is being added
	// more than once.

	std::vector<double> numvoters_options = {1, 100, 10000};

	test_generator_groups grps;

	for (double numvoters: numvoters_options) {
		for (auto & constraint : relative_constraints) {
			std::vector<test_instance_generator> test_generators =
				get_all_permitted_test_generators(numvoters,
					canonical_full_v, *constraint, cand_equivs,
					randomizer);

			for (test_instance_generator itgen : test_generators) {
				grps.insert(itgen);
			}
		}
	}

	// Because the output file is linear, we need to allocate space for
	// the same number of functions no matter what the number of
	// candidates is. So allocate enough to always have room, i.e.
	// as many as the max i: functions_to_test[i].size();
	size_t max_num_functions = 0;
	for (i = min_numcands; i <= max_numcands; ++i) {
		max_num_functions = std::max(max_num_functions,
			functions_to_test[i].size());
	}


	// And now for some tests. Quick and dirty.

	std::vector<test_results> all_results;

	for (size_t i = 0; i < grps.groups.size(); ++i) {

		std::string fn_prefix = settings.test_storage_prefix + itos(i) + ".dat";

		int num_tests = settings.num_tests;
		test_results results(num_tests, max_num_functions);
		results.allocate_space(fn_prefix);

		all_results.push_back(results); // for i
	}

	// Verify meta here.
	// Replace with something better once we have group_order and
	// desired_criteria implemented. TODO.

	backtracker verifier(min_numcands, max_numcands);
	verifier.set_algorithms(functions_to_test);

	std::list<size_t> group_order = settings.group_order;
	if (group_order.empty()) {
		std::cout << "Group order not specified. Generating...\n";
		group_order = get_group_order(grps, all_results, verifier,
			functions_to_test, true);
		std::cout << "Insert the following into the config file " <<
			"to skip this step the next time:\n";
		print_group_order(group_order);
	}

	// Print the order we decided upon.

	for (int ts : group_order) {
		std::cout << ts << ": ";
		grps.groups[ts].print_scenarios(cout);
		std::cout << "\n";
	}

	std::vector<int> cur_results_method_indices(4, -1);
	std::map<copeland_scenario, int> set_algorithm_indices;

	verifier.set_tests_and_results(group_order, grps, all_results);
	verifier.try_algorithms();

	return 0;
}
