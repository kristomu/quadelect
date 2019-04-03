#pragma once

// Configuration file class for general_rpn tools (conmpositor and verifier)
// This makes it easier to specify different settings for different
// projects without having to keep track of which projects use which
// settings.

#include <list>
#include <string>
#include <vector>
#include <stdexcept>
#include <libconfig.h++>

class g_rpn_config {
	public:
		std::vector<std::string> source_files;
		std::vector<std::string> desired_criteria;

		// What order to recurse into the various test_generator_groups.
		// This is important because some criteria are harder to satisfy
		// than others, If the group order isn't specified, the program
		// will try to construct one (and output it at the start of
		// runtime).
		// It's a list for compatibility reasons.
		std::list<int> group_order;

		int num_tests;
		std::string test_storage_prefix;

		void clear();
		void load_from_file(std::string config_filename);
};