std::list<int> exhaustive_method::get_council(int council_size,
	int num_candidates, const election_t & ballots) {

	process_ballots(ballots);

	std::vector<size_t> v(num_candidates);
	std::iota(v.begin(), v.end(), 0);

	// This is a bit hacky, but necessary due to how
	// for_each_combination works. We clone ourselves,
	// pass it as a function, and then this clone carries
	// out all the calculations and returns it through
	// operator().
	shunt this_shunt;
	this_shunt.x = this;

	current_optimum = for_each_combination(v.begin(),
			v.begin() + council_size, v.end(), this_shunt);

	std::list<int> out;

	// FIX LATER
	for (size_t i: current_optimum.get_optimal_solution()) {
		out.push_back(i);
	}
}