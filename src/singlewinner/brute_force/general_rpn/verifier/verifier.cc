#include "../composition/groups/test_generator_groups.h"
#include "../composition/groups/test_generator_group.h"

#include "../composition/test_results.h"

#include "../../../../linear_model/constraints/relative_criterion_producer.h"
#include "../../../../linear_model/constraints/relative_criteria/mono-raise.h"
#include "../../../../linear_model/constraints/relative_criteria/mono-add-top.h"

#include <iostream>
#include <iterator>
#include <memory>

#include <time.h>

#include "../isda.cc"

// This program takes files generated by polytope_compositor to determine
// what combinations of algorithms for the different scenarios produce
// viable election methods (i.e. ones that pass the tests tested by
// polytope_compositor).

// The general idea is to construct a DAG of each polytope_compositor
// test_generator_group, where test group A has an edge to B if all the
// scenarios that appear in A also appear in B. In addition, every node
// has an edge to a supersink.

// We can then tentatively assign a method to each scenario while
// traversing the DAG in topological order, backtracking whenever it's
// clear that the current (partial) assignment fails the test. By starting
// with simple tests first, this strategy should divide-and-conquer pretty
// nicely.

// Next to do, optimization wise: replace the map in backtracker with a
// vector of ints, and have the testing function just do a lookup, e.g.
// chosen[0] = algorithm for 37,4
// chosen[1] = algorithm for 39,4
// test pointer vector for some test group with 37,4/39,4/37,4/39,4 = 0 1 0 1
// starts[i] = chosen[test_pointer_vector[our group][election type]]
// then the rest as before.
// This would also allow us to constrain e.g. 37,4 = 39,4 if searching the
// whole space becomes too slow.

// But probably refactoring before that, and probably changing what make tool
// to use before *that*.

// Cut and paste code, fix later! But where should I put it?
std::vector<test_instance_generator> get_all_permitted_test_generators(
	double max_numvoters,
	const std::vector<copeland_scenario> canonical_scenarios,
	const relative_criterion_const & relative_criterion,
	const fixed_cand_equivalences before_cand_remapping,
	const fixed_cand_equivalences after_cand_remapping,
	rng & randomizer) {

	std::vector<test_instance_generator> out;

	size_t numcands = canonical_scenarios[0].get_numcands();

	for (copeland_scenario x: canonical_scenarios) {
		for (copeland_scenario y: canonical_scenarios) {
			test_generator cur_test(randomizer.long_rand());

			std::cout << "Combination " << x.to_string() << ", "
				<< y.to_string() << ":";
			if (!cur_test.set_scenarios(x, y, max_numvoters,
				relative_criterion)) {
				std::cout << "not permitted\n";
				continue;
			}

			std::cout << "permitted\n";

			// Warning: take note of that numcands might vary for
			// e.g. cloning or ISDA.
			for (size_t i = 1; i < numcands; ++i) {
				test_instance_generator to_add(cur_test);
				// Set a different seed but use the same sampler and
				// polytope as we created earlier.
				to_add.tgen.set_rng_seed(randomizer.long_rand());

				// Get the scenarios by sampling once.
				relative_test_instance ti = to_add.tgen.
					sample_instance(i, before_cand_remapping,
						after_cand_remapping);
				to_add.before_A = ti.before_A.scenario;
				to_add.before_B = ti.before_B.scenario;
				to_add.after_A = ti.after_A.scenario;
				to_add.after_B = ti.after_B.scenario;

				to_add.cand_B_idx = i;
				out.push_back(to_add);
			}
		}
	}
	return out;
}



// Returns true if all scenarios in smaller are also used in larger,
// otherwise false. Ties (same scenarios in both) are broken in a way
// that doesn't produce cycles.
bool uses_subset_of_scenarios(const test_generator_group & smaller,
	const test_generator_group & larger) {

	std::set<copeland_scenario> in_smaller = smaller.get_tested_scenarios(),
		in_larger = larger.get_tested_scenarios();

	for (const copeland_scenario small_cand : in_smaller) {
		if (in_larger.find(small_cand) == in_larger.end()) {
			return false;
		}
	}

	// If they're equal and have the same scenarios, do a minimum tiebreak.
	if (in_smaller.size() == in_larger.size()) {
		return smaller < larger;
	}

	return true;
}

// Depth first search

const int NO_VISIT = 0, TEMP_VISIT = 1, PERM_VISIT = 2;

void toposort_visit(int in_idx,
	const std::vector<std::vector<bool> > & adjacencies,
	std::vector<int> & node_color, std::list<int> & sorted_order) {

	if (node_color[in_idx] == PERM_VISIT) { return; }
	if (node_color[in_idx] == TEMP_VISIT) {
		throw std::runtime_error("toposort: input graph is not a DAG!");
	}

	node_color[in_idx] = TEMP_VISIT;
	for (size_t out_idx = 0; out_idx < adjacencies.size(); ++out_idx) {
		if (adjacencies[in_idx][out_idx]) {
			toposort_visit(out_idx, adjacencies, node_color,
				sorted_order);
		}
	}
	node_color[in_idx] = PERM_VISIT;
	sorted_order.push_front(in_idx);
}

std::list<int> toposort(
	const std::vector<std::vector<bool> > & adjacencies) {

	std::vector<int> node_color(adjacencies.size(), NO_VISIT);
	std::list<int> sorted_order;

	for (size_t i = 0; i < adjacencies.size(); ++i) {
		if (node_color[i] == NO_VISIT) {
			toposort_visit(i, adjacencies, node_color, sorted_order);
		}
	}

	return sorted_order;
}

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
		double get_progress(size_t test_group_idx,
			test_election current_election_setting) const;

		void print_progress(size_t test_group_idx,
			test_election current_election_setting) const;

	public:
		std::vector<test_and_result> tests_and_results;
		std::map<copeland_scenario, int> algorithm_for_scenario;
		std::vector<std::vector<algo_t> > prospective_functions;
		std::vector<std::vector<int> > algorithm_per_setting;
		int numcands;

		// for showing a progress report without wasting too much time
		// on time-elapsed calls.
		size_t global_counter, counter_threshold;
		time_t last_shown_time, start_time;

		// for calculating the progress
		// needs to be improved.
		std::vector<std::vector<int> > max_num_algorithms;

		void set_tests_and_results(
			const std::list<int> & order,
			const test_generator_groups & all_groups,
			const std::vector<test_results> & all_results);

		void try_algorithms(size_t test_group_idx,
			test_election current_election_setting);

		void try_algorithms() {
			start_time = time(NULL);
			try_algorithms(0, TYPE_A);
		}

		backtracker(int numcands_in) {
			numcands = numcands_in; // remove later
			counter_threshold = 1000;
			global_counter = 0;
			last_shown_time = 0;
		}
};

double backtracker::get_progress(size_t test_group_idx,
	test_election current_election_setting) const {

	// This is basically a least significant digit first radix conversion.
	// While MSD is easier to write, it would also cause overflows.
	// idx is indexed backwards to avoid problems with going below zero
	// on an unsigned counter.

	double progress = 0;

	for (size_t idx = 0; idx <= test_group_idx; ++idx) {
		test_election max_current_election_setting = TYPE_B_PRIME;
		if (idx == 0) {
			max_current_election_setting = current_election_setting;
		}

		for (int i = (int)max_current_election_setting; i >= 0; --i) {
			progress += algorithm_per_setting[test_group_idx-idx][i];
			progress /= max_num_algorithms[test_group_idx-idx][i];
		}
	}

	return progress;
}

std::string format_time(double seconds_total) {
	// Use doubles to handle extreme values, e.g. 1e+40 seconds.
	double secs = fmod(seconds_total, 60);
	double x = (seconds_total - secs) / 60.0;
	double mins = fmod(x, 60);
	x = (x - mins) / 60.0;
	double hrs = fmod(x, 24);
	x = (x - hrs) / 24.0;
	double days = fmod(x,365.24);
	x = (x - days) / 365.24;
	double years = x;

	std::string hstr = itos(round(hrs), 2),
		mstr = itos(round(mins), 2), sstr = itos(round(secs), 2);

	// Set up time string.
	std::string time_str;
	if (hrs > 0) {
		time_str = hstr + ":" + mstr + ":" + sstr;
	}
	if (hrs == 0 && mins > 0) {
		time_str = mstr + ":" + sstr;
	}
	if (hrs == 0 && mins == 0) {
		time_str = sstr + "s";
	}

	// Set up date str
	string date_str = "";

	if (days > 0) {
		date_str = itos(round(days), 2) + "d and ";
	}
	if (years > 0) {
		date_str = dtos(years) + "y, " + date_str;
	}

	return date_str + time_str;
}

void backtracker::print_progress(size_t test_group_idx,
	test_election current_election_setting) const {

	double progress = get_progress(test_group_idx, current_election_setting);
	double eta_seconds = (last_shown_time-start_time)/progress;

	std::cerr << "Progress: " << progress << "    ";
	std::cerr << "ETA: " << format_time(eta_seconds) << ".";
	std::cerr << "    \r" << std::flush;
}

void backtracker::try_algorithms(size_t test_group_idx,
	test_election current_election_setting) {

	if (test_group_idx == tests_and_results.size()) {
		// End case.
		std::cout << "Reached the end." << std::endl;
		gen_custom_function evaluator(numcands);

		for (const auto & kv : algorithm_for_scenario) {
			algo_t algorithm = prospective_functions[numcands][kv.second];
			evaluator.set_algorithm(algorithm);
			std::cout << "\t" << kv.first.to_string() << ": "
				<< algorithm << "\t" << evaluator.to_string() << "\n";
		}

		std::cout << std::endl;
		return;
	}

	// First check if we've set an algorithm for the current test group.
	// If not, go through every possible algorithm.

	copeland_scenario current_scenario = tests_and_results[test_group_idx].
		test_group.get_scenario(current_election_setting);

	size_t i;

	if (algorithm_for_scenario.find(current_scenario) ==
		algorithm_for_scenario.end() ||
		algorithm_for_scenario.find(current_scenario)->second == -1) {

		// Go through every possible algorithm. For each, recurse back with
		// the current position the same so we'll fall through next time.

		for (i = 0; i < prospective_functions[numcands].size(); ++i){

			algorithm_for_scenario[current_scenario] = i;

			try_algorithms(test_group_idx, current_election_setting);

			assert(algorithm_for_scenario[current_scenario] == (int)i);
		}

		// Since we've looped through to ourselves, there's no need
		// to do anything but reset the scenario to undecided and return.

		algorithm_for_scenario[current_scenario] = -1;
		return;
	}

	// If we got here, the algorithm to use for the current scenario has
	// already been defined. So set the algorithm_per_setting array to
	// this particular algorithm, as the testing function need is in that
	// particular format.

	// Note that we need a different algorithm_per_setting array for each
	// test_group_idx. Otherwise recursions further in might scribble on
	// an algorithm_per_setting that a recursion further out needs to
	// preserve.

	algorithm_per_setting[test_group_idx][(int)current_election_setting] =
		algorithm_for_scenario[current_scenario];

	// Set the denominator for the progress indicator.
	// This could be done once and for all outside of the loop.
	// TODO: Do that.
	max_num_algorithms[test_group_idx][(int)current_election_setting] =
		prospective_functions[numcands].size();

	// Show a progress report if the global counter is high enough and enough
	// time has elapsed. (1s hard-coded.)
	if (global_counter++ >= counter_threshold) {
		global_counter = 0;
		if (time(NULL) >= last_shown_time + 1) {
			last_shown_time = time(NULL);
			print_progress(test_group_idx, current_election_setting);
		}
	}

	// If we're at the last election setting, run a test, because we have
	// algorithms for every selection setting (A, B, A', B').
	if (current_election_setting == TYPE_B_PRIME) {
		bool pass = tests_and_results[test_group_idx].group_results.
			passes_tests(algorithm_per_setting[test_group_idx]);

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
	const std::list<int> & order,
	const test_generator_groups & all_groups,
	const std::vector<test_results> & all_results) {

	tests_and_results.clear();

	for (int idx: order) {
		tests_and_results.push_back(
			test_and_result(all_groups.groups[idx], all_results[idx]));
	}

	algorithm_per_setting =
		std::vector<std::vector<int> >(tests_and_results.size(),
			std::vector<int>(NUM_REL_ELECTION_TYPES, -1));

	max_num_algorithms = std::vector<std::vector<int> >(
		tests_and_results.size(),
			std::vector<int>(NUM_REL_ELECTION_TYPES, -1));
}

int main(int argc, char ** argv) {
	int numcands = 4;
	rng randomizer(1);

	// Read algorithms from file.

	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " [file containing "
		"sifter output, " << numcands << " cands]" << std::endl;
		return(-1);
	}

	std::string filename = argv[1];
	std::ifstream sifter_file(filename);

	if (!sifter_file) {
		std::cerr << "Could not open " << filename << std::endl;
	}

	std::vector<std::vector<algo_t> > prospective_functions(5);

	get_first_token_on_lines(sifter_file, prospective_functions[numcands]);

	std::vector<algo_t> functions_to_test = prospective_functions[numcands];

	std::cout << functions_to_test.size() << std::endl;

	sifter_file.close();

	// eof

	std::map<int, fixed_cand_equivalences> cand_equivs =
		get_cand_equivalences(4);

	std::set<copeland_scenario> canonical_full = get_nonderived_scenarios(
		numcands, cand_equivs.find(numcands)->second);

	std::vector<copeland_scenario> canonical_full_v;
	std::copy(canonical_full.begin(), canonical_full.end(),
		std::back_inserter(canonical_full_v));

	// We need at least one relative constraint to make our groups, so
	// allocate them here.

	std::vector<std::unique_ptr<relative_criterion_const> >
		relative_constraints;

	// Add some relative constraints. (Kinda ugly, but what can you do.)
	relative_constraints = relative_criterion_producer().get_all(
		3, 4, true);

	// Create all the groups
	// There seem to be some bugs where the same group is being added
	// more than once.

	std::vector<double> numvoters_options = {1, 100, 10000};

	test_generator_groups grps;

	for (double numvoters: numvoters_options) {
		for (auto & constraint : relative_constraints) {
			std::vector<test_instance_generator> test_generators =
				get_all_permitted_test_generators(numvoters,
					canonical_full_v, *constraint,
					cand_equivs.find(numcands)->second,
					cand_equivs.find(numcands)->second,
					randomizer);

			for (test_instance_generator itgen : test_generators) {
				grps.insert(itgen);
			}
		}
	}

	size_t num_groups = grps.groups.size();

	// Create the adjacency matrix
	std::vector<std::vector<bool> > subproblem(num_groups+1,
		std::vector<bool>(num_groups+1, false));
	size_t from, to;

	// Link every node to the supersink that represents printing out the
	// result.

	for (from = 0; from < num_groups; ++from) {
		subproblem[from][num_groups] = true;
	}

	// Link nodes to greater tests.

	for (from = 0; from < num_groups; ++from) {
		for (to = 0; to < num_groups; ++to) {
			subproblem[from][to] = uses_subset_of_scenarios(
				grps.groups[from], grps.groups[to]);
		}
	}

	// Get topologically sorted ordering

	std::list<int> toposorted = toposort(subproblem);

	// Remove the supersink.
	toposorted.pop_back();

	// Print.

	for (int ts : toposorted) {
		std::cout << ts << ": ";
		grps.groups[ts].print_scenarios(cout);
		std::cout << "\n";
	}

	// And now for some tests. Quick and dirty.

	std::vector<test_results> all_results;

	for (size_t i = 0; i < grps.groups.size(); ++i) {

		std::string fn_prefix = "algo_testing/" + itos(i) + "_" + "out.dat";

		int num_tests = 100;
		test_results results(num_tests, prospective_functions[4].size());
		results.allocate_space(fn_prefix);

		all_results.push_back(results); // for i
	}

	// Verify meta here.

	std::list<int> only = {14, 15};
	std::list<int> trunc = toposorted;
	//std::cout << *toposorted.begin() << std::endl;

	std::vector<int> cur_results_method_indices(4, -1);
	std::map<copeland_scenario, int> set_algorithm_indices;

	backtracker foo(4);
	foo.set_tests_and_results(trunc, grps, all_results);
	foo.prospective_functions = prospective_functions;
	foo.try_algorithms();

	return 0;
}