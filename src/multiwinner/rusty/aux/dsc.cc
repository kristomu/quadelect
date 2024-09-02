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
void simple_dsc(const election_t input, size_t num_candidates) {

	std::multimap<double, std::set<unsigned short> > ranked_sets = get_dsc(
			input);

	std::set<int> hopefuls;

	for (size_t counter = 0; counter < num_candidates; ++counter) {
		hopefuls.insert(counter);
	}

	bool debug = false;

	for (std::map<double, std::set<unsigned short> >::const_reverse_iterator
		p = ranked_sets.rbegin(); p != ranked_sets.rend() &&
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

template<typename T, typename R> std::multimap<R, T> multi_invert(
	const std::map<T, R> input) {

	std::multimap<R, T> inverted;

	for (typename std::map<T, R>::const_iterator pos = input.begin();
		pos != input.end(); ++pos) {
		inverted.insert(std::pair<R, T>(pos->second, pos->first));
	}

	return (inverted);
}



std::multimap <double, std::set<unsigned short> > get_solid_coalitions(
	const election_t & input, int num_candidates,
	int tiebreaker) {

	// For each ballot_group,
	// 	For each candidate,
	// 		If the last one had a different score,
	// 			Increase the map from current set by the ballot
	// 			group's	support.
	// 		Add this one to the set
	// 	Increase the map from current set by the ballot group's support.

	std::map<std::set<unsigned short>, double > forwards;

	// Since it's very difficult to break ties in DAC/DSC, we hack it by
	// adding a very small amount of votes to the first voter after
	// "tiebreaker" number of voters. If this is -1, then that feature
	// isn't added. TODO: Find a way of making sure this never alters
	// a non-tie outcome.
	double tie_count = 0;

	double numvoters_pow = 0;

	for (election_t::const_iterator outer_pos = input.begin();
		outer_pos != input.end(); ++outer_pos) {
		std::set<unsigned short> coalition;
		double last_score = INFINITY;

		bool is_tiebreaker = false;
		if (tiebreaker != -1 && tie_count != -1) {
			if (tie_count >= tiebreaker) {
				is_tiebreaker = true;
				tie_count = -1;
			} else {
				tie_count += outer_pos->get_weight();
			}
		}

		numvoters_pow += outer_pos->get_weight();

		for (ordering::const_iterator inner_pos = outer_pos->contents.
				begin(); inner_pos != outer_pos->contents.end();
			++inner_pos) {
			if (inner_pos->get_score() != last_score &&
				!coalition.empty()) {
				forwards[coalition] += outer_pos->get_weight();
				// TODO: Proper epsilon here.
				if (is_tiebreaker) forwards[coalition] +=
						1e-5;
			}

			coalition.insert(inner_pos->get_candidate_num());
			last_score = inner_pos->get_score();
		}

		if (!coalition.empty()) {
			forwards[coalition] += outer_pos->get_weight();
			// Do right next time
			if (is_tiebreaker) {
				forwards[coalition] *= (1 + 1e-5);
			}
		}
	}

	// Construct the set of all candidates. (HACK HACK)
	std::set<unsigned short> all;
	for (int counter = 0; counter < num_candidates; ++counter) {
		all.insert(counter);
	}
	forwards[all] = numvoters_pow;

	return (multi_invert(forwards));
}
