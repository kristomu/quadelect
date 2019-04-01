#include "general_rpn.h"
#include <iostream>

void g_rpn_config::clear() {
	source_files.clear();
	desired_criteria.clear();
	criteria_order.clear();

	num_tests = 0;
	test_storage_prefix = "";
}

void g_rpn_config::load_from_file(std::string config_filename) {
	libconfig::Config cfg;

	// TODO?? somehow report the error in a throw instead of writing
	// directly to cerr?
	try {
		cfg.readFile(config_filename.c_str());
	} catch (libconfig::FileIOException fioex) {
		std::cerr << "Error reading config file " << config_filename << "\n";
		throw std::runtime_error("Config file read error");
	} catch (libconfig::ParseException pex) {
		std::cerr << "Error parsing config file " << config_filename << "\n";
		std::cerr << pex.getFile() << ": " << pex.getLine() << " - "
			<< pex.getError() << "\n";
		throw std::runtime_error("Config file read error");
	}

	int i;

	clear();

	const libconfig::Setting & root = cfg.getRoot();
	const libconfig::Setting & algosearch = root["algorithms_search"];

	// Dump input source file names.
	for (i = 0; i < algosearch["source_files"].getLength(); ++i) {
		source_files.push_back(algosearch["source_files"][i]);
	}

	// Dump desired criteria
	for (i = 0; i < algosearch["desired_criteria"].getLength(); ++i) {
		desired_criteria.push_back(algosearch["desired_criteria"][i]);
	}

	// and order (if specified).
	try {
		for (i = 0; i < algosearch["criteria_order"].getLength(); ++i) {
			desired_criteria.push_back(algosearch["criteria_order"][i]);
		}
	} catch (libconfig::SettingNotFoundException snfex) {
		// ignore
	}
	
	algosearch.lookupValue("num_tests", num_tests);
	algosearch.lookupValue("test_storage_prefix", test_storage_prefix);
}
