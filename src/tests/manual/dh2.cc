#include "dh2.h"

dh2_test::dh2_test() {

	honest = parser.interpret_ballots( {
		"50000: A>B>C",
		"50000: B>A>C",
		"    1: C>A>B",
		"    1: C>B>A"}, false);

	A_buries = parser.interpret_ballots( {
		"50000: A>C>B",
		"50000: B>A>C",
		"    1: C>A>B",
		"    1: C>B>A"}, false);

	B_buries = parser.interpret_ballots( {
		"50000: A>B>C",
		"50000: B>C>A",
		"    1: C>A>B",
		"    1: C>B>A"}, false);

	both_bury = parser.interpret_ballots( {
		"50000: A>C>B",
		"50000: B>C>A",
		"    1: C>A>B",
		"    1: C>B>A"}, false);
}

std::set<std::string> get_method_winners(const election_method & method,
	const std::pair<std::map<size_t, std::string>, election_t> &
	election_and_names, size_t numcands) {

	ordering outcome = method.elect(election_and_names.second,
			numcands, true);

	std::vector<int> winners = ordering_tools::get_winners(outcome);

	std::set<std::string> winning_cands;

	for (int winner: winners) {
		winning_cands.insert(election_and_names.
			first.find(winner)->second);
	}

	return winning_cands;
}

// TODO, overload instead??? See ref implementation of vse for how
bool elects(const std::set<std::string> & haystack, std::string needle) {
	return (haystack.find(needle) != haystack.end());
}

bool dh2_test::passes(election_method & method, bool verbose) const {
	std::set<std::string> w_honest = get_method_winners(method,
			honest, 3);

	std::string prefix = "DH2 pass: " + method.name() + ": ";

	// If we elect only one of A and B, or neither, then at least
	// one faction has no incentive to bury.
	// Thus we have a pass.
	if (!elects(w_honest, "A") || !elects(w_honest, "B")) {
		if (verbose) {
			std::cout << prefix << "no initial reason to bury.\n";
		}
		return true;
	}

	// if A's burial doesn't turn B from a winner to a loser, then pass
	std::set<std::string> w_Abury = get_method_winners(method,
			A_buries, 3);

	if (!elects(w_honest, "B") || elects(w_Abury, "B")) {
		if (verbose) {
			std::cout << prefix << "doesn't reward burial by A.\n";
		}
		return true;
	}

	// if B's burial doesn't turn A from a winner to a loser, then pass
	std::set<std::string> w_Bbury = get_method_winners(method,
			B_buries, 3);

	if (!elects(w_honest, "A") || elects(w_Bbury, "A")) {
		if (verbose) {
			std::cout << prefix << "doesn't reward burial by B.\n";
		}
		return true;
	}

	// if both burying doesn't elect C, then pass
	std::set<std::string> w_both = get_method_winners(method,
			both_bury, 3);

	if (!elects(w_both, "C")) {
		if (verbose) {
			std::cout << prefix << "doesn't break down when both "
				"bury\n";
		}
		return true;
	}

	// otherwise fail.
	if (verbose) {
		std::cout << "DH2 --failure-- " << method.name() << "\n";
	}

	return false;
}
