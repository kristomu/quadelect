#pragma once

// Configuration file class for general_rpn tools (conmpositor and verifier)
// This makes it easier to specify different settings for different
// projects without having to keep track of which projects use which
// settings.

#include <string>
#include <vector>
#include <stdexcept>
#include <libconfig.h++>

class g_rpn_config {
	public:
		std::vector<std::string> source_files;
		std::vector<std::string> desired_criteria;

		// Which criteria should be tested first? If empty, it doesn't
		// matter.This is important most of the time because some criteria
		// are harder to satisfy than others, and so those should be put
		// first so that backtracking happens early.
		std::vector<std::string> criteria_order;
		int num_tests;
		std::string test_storage_prefix;

		void clear();
		void load_from_file(std::string config_filename);
};