#include "test_generator_group.h"

// Sample a ballot from the given test_instance_generator and return a
// vector test instance in such a way that the scenarios fit with the
// specified scenarios for the test_generator_group. 

// There are two ways to do so: straightforward, if the test instance
// generator has the same scenarios (before_A == group's before_A and
// so on), or reversed.

// Reversed makes use of that a given set of algorithms pass a test
// instance for a relative criterion if

// score(eA) - score(eB) <= score(eA') - score(eB')

// where eA, eB, eA', eB' are the elections before alteration from A and B's
// perspective, and after alteration from A and B's perspective respectively.
// This can be rewritten as:

// score(eA) - score(eB) - score(eA') + score(eB') <= 0
// score(eB') - score(eA') <= score(eB) - score(eA)

// so by turning eB' into eA, eA' into eB, eB into eA', and eA into eB',
// the test will hold iff the original test held. By doing so, the
// respective scenarios are also swapped around.

vector_test_instance test_generator_group::sample(
	test_instance_generator generator, bool reverse, 
	const std::map<int, fixed_cand_equivalences> & 
	candidate_equivalences) const {

	relative_test_instance ti = generator.get_test_instance(
			candidate_equivalences);

	if (reverse) {
		relative_test_instance rti;

		rti.before_A = ti.after_B;
		rti.before_B = ti.after_A;
		rti.after_A = ti.before_B;
		rti.after_B = ti.before_A;

		ti = rti;
	}

	return vector_test_instance(ti);
}

std::vector<vector_test_instance> test_generator_group::sample(
	size_t desired_samples, const std::map<int, fixed_cand_equivalences> & 
	candidate_equivalences) {

	if (desired_samples < generators.size()) {
		throw std::runtime_error(
			"test_generator_group: too few samples to cover all "
			"test instances.");
	}

	if (generators.empty()) {
		throw std::runtime_error(
			"test_generator_group: tried to sample from empty group.");
	}

	size_t sample_number = 0;

	std::vector<vector_test_instance> elections;
	elections.reserve(desired_samples);

	while (sample_number != desired_samples) {
		for (size_t i = 0; i < generators.size(); ++i) {
			if (sample_number++ == desired_samples) { 
				return elections;
			}

			elections.push_back(sample(generators[i], 
				should_be_reversed[i], candidate_equivalences));
		}
	}

	return elections;
}

bool test_generator_group::fits_group_directly(
	const test_instance_generator & candidate) const {
	// If it's empty, everything matches.
	if (generators.empty()) { return true; }

	// Check if the scenarios match directly.

	return candidate.before_A == before_A && candidate.after_A == after_A &&
			candidate.before_B == before_B && candidate.after_B == after_B;
}

bool test_generator_group::fits_group_reversed(
	const test_instance_generator & candidate) const {

	if (generators.empty()) { return true; }

	// Check if the scenarios match after reversing (see beginning of file).

	return candidate.after_B == before_A && candidate.after_A == before_B &&
		candidate.before_B == after_A && candidate.before_A == after_B;
}

bool test_generator_group::fits_group(
	const test_instance_generator & candidate) const {

	return fits_group_directly(candidate) || fits_group_reversed(candidate);
}

void test_generator_group::insert(test_instance_generator candidate) {
	if (!fits_group(candidate)) {
		throw std::runtime_error(
			"test_generator_group: inserted wrong type of "
			"test_instance_generator!");
	}

	// If the group is empty, we need to set what's allowable from now on.
	if (generators.empty()) {
		before_A = candidate.before_A;
		after_A = candidate.after_A;
		before_B = candidate.before_B;
		after_B = candidate.after_B;
	}

	generators.push_back(candidate);
	should_be_reversed.push_back(fits_group_reversed(candidate) && 
		!fits_group_directly(candidate));
}

void test_generator_group::print_members() const {
	for (test_instance_generator itgen : generators) {
		std::cout << "A: " << itgen.before_A.to_string()
			<< " B: " << itgen.before_B.to_string()
			<< " A': " << itgen.after_A.to_string()
			<< " B': " << itgen.after_B.to_string() << "\t"
			<< "cddt B = # " << itgen.cand_B_idx << "\n";
	}
}

void test_generator_group::print_scenarios(std::ostream & where) const {
	where << before_A.to_string() << " " << before_B.to_string() << " "
		<< after_A.to_string() << " " << after_B.to_string();
}