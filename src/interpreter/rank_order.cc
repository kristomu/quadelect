
// This is the interpreter class for ballots of the form
//	[number]: cand (> or =) cand ... cand
// or just        cand (> or =) cand ... cand

#include "rank_order.h"
#include <iostream>

bool rank_order_int::is_this_format(const std::vector<std::string> &
	inputs) const {

	// if the first line is RANK_ORDER, we know it's this format.
	if (inputs.empty()) {
		return (false);
	}
	if (inputs[0] == "RANK_ORDER") {
		return (true);
	}

	// Otherwise, only alnums, =, :, ., and > are permitted (as well as the
	// obvious newlines and spaces). It must also have at least one alnum.

	// Possible todo later: more sophisticated checking, such as no rank
	// symbols before a :.

	for (size_t line = 0; line < inputs.size(); ++line) {
		for (size_t sec = 0; sec < inputs[line].size(); ++sec) {
			bool accepted = false;

			switch (inputs[line][sec]) {
				case ' ':
				case ':':
				case '>':
				case '=':
				case 0x13:
				case 0x10:
					accepted = true;
					break;
				// Used to be isalnum, but we should
				// also support things like
				// "Write-In".
				default:
					if (isprint(inputs[line][sec])) {
						accepted = true;
					} else {
						accepted = false;
					}
					break;
			}

			if (!accepted) {
				std::cerr << "Not accepted" << std::endl;
				std::cerr << "\t" << inputs[line] << " at " <<
					inputs[line][sec] << std::endl;
				return (false);
			}
		}
	}

	return (true);
}

names_and_election rank_order_int::interpret_ballots(
	const std::vector<std::string> & inputs, bool debug) const {

	// For each line: If it's RANK_ORDER, ignore. Otherwise, check if
	// there's a : preceded by only numbers (or . in case it's decimal).
	// If so, the stuff before the : goes into dtos to find the weight and
	// we set the new start to just after :, otherwise we set the new_start
	// to the beginning.

	// From new_start on, it's simple. Read in chars until we find > or =.
	// Remove spaces from the ends of the candidate names and look the names
	// up. If there's no hit, allocate the next available candidate number
	// to it. In either case, after this is done, add the ballot entry with
	// the proper name and rank to the ballot, and repeat until the line is
	// done.

	std::map<std::string, size_t> fwd;
	names_and_election to_ret;
	int candidates = 0;

	if (debug) {
		std::cout << "Interpreting...." << std::endl;
		std::cout << "Input size: " << inputs.size() << std::endl;
	}

	int min_rank = 0;

	for (size_t counter = 0; counter < inputs.size(); ++counter) {
		if (inputs[counter] == "RANK_ORDER") {
			continue;
		}

		if (debug) {
			std::cout << "Debug parse: " << inputs[counter] << std::endl;
		}

		size_t sec;
		// Find the : if there is one.
		int num_end = -1;

		ballot_group to_add;

		for (sec = 0; sec < inputs[counter].size() && num_end == -1;
			++sec)
			if (inputs[counter][sec] == ':') {
				num_end = sec;
			}

		double weight = 1;
		// If we found something, then hi-ho hi-ho to the stod you go.
		if (num_end > 0)
			weight = str_tod(inputs[counter].substr(0,
						num_end));

		assert(weight > 0);

		to_add.set_weight(weight);

		// Starting from either 0 or after the :...
		int cur_rank = 0;
		while (num_end+1 < (int)inputs[counter].size()) {
			std::string cur_name;
			int term = -1;
			min_rank = std::min(min_rank, cur_rank);
			for (sec = num_end+1; sec < inputs[counter].size() &&
				term == -1; ++sec) {
				// Slurp letters to form the current candidate
				// name until we reach >, =, or end of line.

				switch (inputs[counter][sec]) {
					case '>':
					case '=':
						term = sec;
						break;
					default:
						cur_name +=
							inputs[counter][sec];
						break;
				}
			}

			// Remove the excess spaces from the candidate name, if
			// any.
			int first_nonspace_before, first_nonspace_after;
			for (sec = 0; sec < cur_name.size() &&
				cur_name[sec] == ' '; ++sec);
			first_nonspace_before = sec;
			// Nasty trick: sec <= cur_name.size()-1 really means >= 0
			// because subtracting one from 0 will wrap around to max
			// value for size_t.
			for (sec = cur_name.size()-1; sec <= cur_name.size()-1 &&
				cur_name[sec] == ' '; --sec);
			first_nonspace_after = sec;

			cur_name = cur_name.substr(first_nonspace_before,
					first_nonspace_after + 1 -
					first_nonspace_before);

			// Look up this candidate's candidate number. If there
			// is none, add a new number.
			// TODO: ignore_case parameter. If it's on, it would
			// interpret GORE and gOre equally.

			int cand_number;
			if (fwd.find(cur_name) == fwd.end()) {
				cand_number = candidates++;
				fwd[cur_name] = cand_number;
			} else {
				cand_number = fwd.find(cur_name)->second;
			}

			// Dump this preference into the ballot we're setting.
			to_add.contents.insert(candscore(cand_number,
					cur_rank));

			// If our stopping point wasn't EOL, it could be either
			// > or =. If it's >, we should decrease rank so that
			// the next candidate has lower rank, otherwise we
			// shouldn't.
			if (term != -1 && inputs[counter][term] == '>') {
				--cur_rank;
			}

			// Finally, update stopping point.
			if (term != -1) {
				num_end = term;
			} else	{
				num_end = inputs[counter].size()-1;
			}
		}

		// If there was a ballot there, add it.
		// (???: Do this anyway to handle "Irrelevant ballots"? Perhaps
		//  only when a weight has been explicitly stated.)
		if (!to_add.contents.empty()) {
			to_ret.second.push_back(to_add);
		}
	}

	// Invert the fwd map.
	to_ret.first = invert(fwd);

	// Postprocess the ballots so the ratings are all positive (seems to
	// make a diff for median, etc).
	for (election_t::iterator lbpos = to_ret.second.begin();
		lbpos != to_ret.second.end(); ++lbpos) {
		ordering replacement;

		for (ordering::const_iterator opos = lbpos->contents.begin();
			opos != lbpos->contents.end(); ++opos)
			replacement.insert(candscore(opos->
					get_candidate_num(),
					opos->get_score() - min_rank));

		lbpos->contents = replacement;
	}

	// And done!

	return (to_ret);
}
