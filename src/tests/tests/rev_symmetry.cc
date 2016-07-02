// Class for the reversal symmetry two-test: A method fails reversal symmetry
// if there's a ballot set for which "most liked" (everybody votes in the order
// specified) gives the same result/winner as "most hated" (everybody votes in
// reverse order).

// This is not a good thing for multiwinner elections. See WDS.

#include "../two_tests.h"
#include "rev_symmetry.h"

using namespace std;

pair<bool, list<ballot_group> > test_reversal_symmetry::rearrange_ballots(
		const list<ballot_group> & input, int numcands,
		const vector<int> & data) const {

	// Very simple (if perhaps slow) - just insert the candidates with
	// negated scores.
	
	pair<bool, list<ballot_group> > modified;

	for (list<ballot_group>::const_iterator pos = input.begin(); pos !=
			input.end(); ++pos) {
		ballot_group to_insert;
		to_insert.weight = pos->weight;
		to_insert.contents = otools.reverse(pos->contents);

		modified.second.push_back(to_insert);
	}

	modified.first = true;

	return(modified);
}

bool test_reversal_symmetry::applicable(const ordering & check,
		const vector<int> & data, bool orig) const {

	if (permit_ties)
		return(true);

	// If winner, just check first and last ranks (depending on 
	// raise/lower?), otherwise check all.
	if (winner_only) {
		double first_score = check.begin()->get_score();
		ordering::const_iterator pos;
		// If there's a tie for top, return false (not applicable)
		for (pos = check.begin(); pos != check.end() && pos->get_score()
				== first_score; ++pos)
			if (pos != check.begin()) return(false);
		// Same for last
		double last_score = check.rbegin()->get_score();
		ordering::const_reverse_iterator rpos;
		for (rpos = check.rbegin(); rpos != check.rend() && rpos->
				get_score() == last_score; ++rpos)
			if (rpos != check.rbegin()) return(false);
	} else {
		// If there's any tie at all, no cookie for you!
		double delta = check.begin()->get_score();
		ordering::const_iterator pos;
		for (pos = check.begin(); pos != check.end(); ++pos) {
			if (pos == check.begin()) continue;
			if (pos->get_score() == delta) return(false);
			delta = pos->get_score();
		}
	}

	return(true);
}

bool test_reversal_symmetry::pass_internal(const ordering & original, 
		const ordering & modified, const vector<int> & data,
		int numcands) const {

	// Now, there are three ways to fail Reversal Symmetry. One may have
	// ordinary: A > B > C
	// reverse:  B > C > A
	// or
	// ordinary: A > B > C
	// reverse:  C > A > B
	// or both at once. Thus we check both.

	ordering revd_modified = otools.scrub_scores(otools.reverse(modified)),
		 orig_modified = otools.scrub_scores(original);

	ordering revd_fwd = otools.scrub_scores(modified),
		 orig_fwd = otools.scrub_scores(otools.reverse(original));

	// If the winners of one isn't equal to the losers of the other or
	// vice versa, then the method doesn't pass, no matter whether
	// we're considering only winners or the entire rank.
	if (otools.get_winners(revd_modified) != otools.
			get_winners(orig_modified))
		return(false);

	if (otools.get_winners(revd_fwd) != otools.get_winners(orig_fwd))
		return(false);

	// If we want a full rank, the winners may still agree, so we have
	// to check that, too. Note that we don't need to check *_fwd because
	// if they're identical, they're identical.

	if (!winner_only)
		return(revd_modified == orig_modified);
	else	return(true);
}

string test_reversal_symmetry::name() const {
	string base = "Reversal Symmetry(";

	if (winner_only)
		base += "winner, ";
	else	base += "rank, ";

	if (permit_ties)
		base += "ties)";
	else	base += "pref)";

	return(base);
}

string test_reversal_symmetry::explain_change_int(const vector<int> & data, 
		const map<int, string> & cand_names) const {

	return("the ballots were reversed");
}
