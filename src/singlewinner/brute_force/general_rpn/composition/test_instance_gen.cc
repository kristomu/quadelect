#include "test_instance_gen.h"

// Get a test instance from a generator and candidate equivalences.
relative_test_instance test_instance_generator::get_test_instance(
	const std::map<int, fixed_cand_equivalences> candidate_equivalences) {

	int before_numcands = before_A.get_numcands(),
		after_numcands = after_A.get_numcands();

	relative_test_instance ti = tgen.sample_instance(cand_B_idx,
			candidate_equivalences.find(before_numcands)->second,
			candidate_equivalences.find(after_numcands)->second);

	return ti;
}