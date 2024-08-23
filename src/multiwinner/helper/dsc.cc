#include "dsc.h"

#include "tools/tools.h"

// DSC stuff, copied from old code
// I have a better implementation elsewhere, but let's focus on reproducing
// the old code first.

std::set<unsigned short> get_candidates(ordering::const_iterator begin,
	ordering::const_iterator ending, bool debug) {

	std::set<unsigned short> toRet;

	for (ordering::const_iterator pos = begin; pos != ending; ++pos) {
		toRet.insert(pos->get_candidate_num());
	}

	if (debug) {
		std::cout << "{ ";
		copy(toRet.begin(), toRet.end(),
			std::ostream_iterator<int>(std::cout, " "));
		std::cout << "}" << std::endl;
	}

	return toRet;
}

std::multimap<double, std::set<unsigned short> > get_dsc(
	const election_t & input) {

	// Descending solid coalitions: Treat a vote for A > B > C > D
	// as one for {A, B, C} (which is the same as {B, C, A} and {C, B, A})
	// > D. Doing so, increment counts for each possible set that the voter
	// is solidly committed to, where solid commitment means that the
	// voter strictly ranks all the candidates above those outside the
	// set.

	// Then go down the sets (in order where the one with the largest count,
	// which is to say the most solidly supporting voters, go first),
	// doing set intersections with the "hopeful candidates" set that
	// starts off with all candidates.

	// Optimization: Don't increment the set if some excluded member is
	// within start and end. This would ensure that no more than a certain
	// amount of memory is used in each round.

	// Assumes complete ballots. We may have to put this into boolean
	// so that it'll abort if the ballots aren't complete.

	// I haven't found a way to get a social ordering from this yet,
	// especially in the case of ties. In the worst case, branches may
	// be required to find the winners.

	std::map<std::set<unsigned short>, double> set_counter;

	bool debug = false;

	for (int size = input.begin()->contents.size()-1; size > 0; --size) {
		for (election_t::const_iterator ballot = input.begin();
			ballot != input.end(); ++ballot) {
			ordering::const_iterator beginning, ending,
					 just_inside;

			beginning = ballot->contents.begin();
			ending = beginning;

			for (int counter = 0; counter < size; ++counter) {
				++ending;
			}

			// Check that it's solidly committed - i.e that the
			// next set member isn't equally ranked with the
			// current one. If it's not... forget it.

			just_inside = ending;
			--just_inside;

			bool solidly_committed = false;

			if (ending == ballot->contents.end()) {
				solidly_committed = true;
			} else	solidly_committed = just_inside->get_score() >
					ending->get_score();

			if (solidly_committed) {
				set_counter[get_candidates(beginning, ending,
												  false)] += ballot->get_weight();
			}
		}
	}

	// Debug
	if (debug) {
		for (std::map<std::set<unsigned short>, double>::const_iterator p =
				set_counter. begin(); p != set_counter.end();
			++p) {
			std::cout << p->second << "\t{";
			copy(p->first.begin(), p->first.end(),
				std::ostream_iterator<int>(std::cout, " "));
			std::cout << "}" << std::endl;
		}
	}

	std::multimap<double, std::set<unsigned short> > out_coalitions;

	for (auto pos = set_counter.begin(); pos != set_counter.end(); ++pos) {
		out_coalitions.insert({pos->second, pos->first});
	}

	return out_coalitions;
}

// No regard for ties. The real way to do this is to branch on encountering
// a tie, then say all the candidates that the various (recursive) functions
// return as winners are tied for first place.
void simple_dsc(const election_t input, int num_candidates) {

	std::multimap<double, std::set<unsigned short> > ranked_sets = get_dsc(
			input);

	std::set<int> hopefuls;

	for (int counter = 0; counter < num_candidates; ++counter) {
		hopefuls.insert(counter);
	}

	bool debug = false;

	for (std::map<double, std::set<unsigned short> >::const_reverse_iterator p
		=
			ranked_sets.rbegin(); p != ranked_sets.rend() &&
		hopefuls.size() > 1; ++p) {
		if (debug) {
			std::cout << "Now checking " << p->first << "\t{";
			copy(p->second.begin(), p->second.end(),
				std::ostream_iterator<int>(std::cout, " "));
			std::cout << "}" << " hopefuls: {";
			copy(hopefuls.begin(), hopefuls.end(),
				std::ostream_iterator<int>(std::cout, " "));
			std::cout << "}";
		}

		// Check if it's a set that would exclude all remaining
		// hopefuls. If it is, ignore it, otherwise do a set
		// intersection.

		if (!includes(p->second.begin(), p->second.end(),
				hopefuls.begin(), hopefuls.end())) {
			if (debug) {
				std::cout << "\tIntersecting.";
			}

			std::set<int> intersect;
			std::set_intersection(hopefuls.begin(), hopefuls.end(),
				p->second.begin(), p->second.end(),
				std::inserter(intersect, intersect.begin()));

			if (!intersect.empty()) {
				hopefuls = intersect;
			}
		} else {
			if (debug) {
				std::cout << "\tIgnoring.";
			}
		}
		if (debug) {
			std::cout << std::endl;
		}
	}
}
