#include "dh2.h"

#include "../../tools/ballot_tools.h"

dh2_test::dh2_test() {

	honest = parser.interpret_ballots({
		"50000: A>B>C",
		"50000: B>A>C",
		"    1: C>A>B",
		"    1: C>B>A"}, false);

	A_buries = parser.interpret_ballots({
		"50000: A>C>B",
		"50000: B>A>C",
		"    1: C>A>B",
		"    1: C>B>A"}, false);

	B_buries = parser.interpret_ballots({
		"50000: A>B>C",
		"50000: B>C>A",
		"    1: C>A>B",
		"    1: C>B>A"}, false);

	both_bury = parser.interpret_ballots({
		"50000: A>C>B",
		"50000: B>C>A",
		"    1: C>A>B",
		"    1: C>B>A"}, false);
}

std::set<std::string> dh2_test::get_method_winners(
	const election_method & method, const names_and_election &
	name_election, size_t numcands) const {

	ordering outcome = method.elect(name_election.second,
			numcands, true);

	std::vector<int> winners = ordering_tools::get_winners(outcome);

	std::set<std::string> winning_cands;

	for (int winner: winners) {
		winning_cands.insert(
			name_election.first.find(winner)->second);
	}

	return winning_cands;
}

bool dh2_test::elects(const std::set<std::string> & haystack,
	std::string needle) const {

	return (haystack.find(needle) != haystack.end());
}

bool dh2_test::passes(election_method & method, bool verbose) const {
	std::set<std::string> w_honest = get_method_winners(method,
			honest, 3);

	// If verbose, we'll output every way that the method passes
	// the criterion; otherwise, we'll stop immediately after it's
	// clear that the method passes.

	bool does_pass = false;

	std::string prefix = "DH2 pass: " + method.name() + ": ";

	// If we elect only one of A and B, or neither, then at least
	// one faction has no incentive to bury.
	// Thus we have a pass.
	if (!elects(w_honest, "A") || !elects(w_honest, "B")) {
		if (verbose) {
			std::cout << prefix << "no initial reason to bury.\n";
			does_pass = true;
		} else {
			return true;
		}
	}

	// if A's burial doesn't turn B from a winner to a loser, then pass
	std::set<std::string> w_Abury = get_method_winners(method,
			A_buries, 3);

	if (!elects(w_honest, "B") || elects(w_Abury, "B")) {
		if (verbose) {
			std::cout << prefix << "doesn't reward burial by A.\n";
			does_pass = true;
		} else {
			return true;
		}
	}

	// if B's burial doesn't turn A from a winner to a loser, then pass
	std::set<std::string> w_Bbury = get_method_winners(method,
			B_buries, 3);

	if (!elects(w_honest, "A") || elects(w_Bbury, "A")) {
		if (verbose) {
			std::cout << prefix << "doesn't reward burial by B.\n";
			does_pass = true;
		} else {
			return true;
		}
	}

	// if both burying doesn't elect C, then pass
	std::set<std::string> w_both = get_method_winners(method,
			both_bury, 3);

	if (!elects(w_both, "C")) {
		if (verbose) {
			std::cout << prefix << "doesn't break down when both "
				"bury\n";
			does_pass = true;
		} else {
			return true;
		}
	}

	if (does_pass) {
		return true;
	}

	// otherwise fail.
	if (verbose) {
		std::cout << "DH2 --failure-- " << method.name() << "\n";
	}

	return false;
}