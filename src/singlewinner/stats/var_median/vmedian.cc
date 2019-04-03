// Median ratings, including the gradual relaxation tiebreaker specified in
// http://wiki.electorama.com/wiki/Median_Ratings . The tiebreaker handles
// weighted ballots with a plane-sweep algorithm.

// The idea of this algorithm is to keep two vertical lines at 50% +/- p,
// p increasing. The vertical lines stop at the closest transition between
// two piecewise constant segments of the sorted ratings function.

#include "vmedian.h"

using namespace std;

grad_fracile vi_median_ratings::aggregate_ratings(
		const list<ballot_group> & papers, int num_candidates, 
		const vector<bool> & hopefuls, bool do_norm, double minimum,
		double maximum) const {

	// Not-hopefuls get zeroes. Hopefuls get rated as the ballot specifies,
	// possibly normalized. If normalizing, we assume integer scores.

	grad_fracile out;
	out.add_candidates(num_candidates);

	double score;

	for (list<ballot_group>::const_iterator pos = papers.begin(); pos !=
			papers.end(); ++pos) {
		
		double local_min = INFINITY, local_max = -INFINITY;
		ordering::const_iterator opos;

		// ??? May cause a problem when we add hopefuls. Say we have
		// 0-10 Range and a "superbad" candidate. He'll help the others
		// get normed correctly even if he's not set as hopeful.
		// I really should make "hopefuls" transparent.
		if (do_norm)
			for (opos = pos->contents.begin(); 
					opos != pos->contents.end(); ++opos) {
				local_min = min(local_min, opos->get_score());
				local_max = max(local_max, opos->get_score());
			}

		for (opos = pos->contents.begin(); opos != pos->contents.end();
			++opos) {

			if (hopefuls[opos->get_candidate_num()]) {
				if (do_norm && local_min != local_max) {
					score = renorm(local_min, local_max,
							opos->get_score(),
							minimum, maximum);
					score = round(score);
				} else	score = opos->get_score();
			} else	score = minimum;

			score = min(maximum, max(minimum, score));

			assert(out.add_rating(opos->get_candidate_num(),
						pos->weight, score));
		}

	}

	// Transport hopefuls over.
	for (size_t counter = 0; counter < hopefuls.size(); ++counter)
		if (!hopefuls[counter])
			out.exclude(counter);

	return(out);
}

pair<ordering, bool> vi_median_ratings::get_ranks(grad_fracile & source,
		double fracile, bool tiebreak, bool winner_only, 
		bool debug) const {

	list<pair<double, int> > l_ordering;

	gf_comparand comparison(&source);
	bool broke_early = false, updated = true;

	// Init the fracile structure. We'll use GF_NONE here for the time
	// being, though it may not be optimal.
	assert(source.init(fracile, GF_NONE));

	list<pair<double, int> >::iterator opos, obpos;
	size_t iter = 0, counter;

	// Create our ordering struct with the initial state of every candidate
	// being tied with each other.
	for (counter = 0; counter < source.get_num_candidates(); ++counter)
		if (source.is_hopeful(counter))
			l_ordering.push_back(pair<double, int>(0, counter));

	// While some boxes are finite, there are ties left, and we haven't
	// broken early, keep on iterating.
	while (source.get_num_hopefuls() > 1 && updated && 
			(tiebreak || iter < 1) && !broke_early) {

		++iter;

		if (debug) {
			cout << "Iter " << iter << endl;

			for (counter = 0; counter < source.get_num_candidates();
					++counter) {
				if (!source.is_hopeful(counter)) continue;

				cout << "Candidate " << counter << ": " << 
					source.get_score(counter)<< "\t left: ";
				source.print_next_pair(counter, true);
				cout << " right: ";
				source.print_next_pair(counter, false);
				cout << endl;
			}
		}

		// Sort the list according to a functor that first considers
		// the ranks and second the current scores. This will break
		// any ties (same rank) that has been resolved by score.

		if (source.ordinal_changed()) {
			if (debug)
				cout << "-Changed-" << endl;
			l_ordering.sort(comparison); // Sort the list.
		}

		// Relabel to integers and find candidates that are no longer
		// tied. We go down the sorted list, either copying the last
		// (integer) rank, or setting the rank to a counter, depending
		// on whether the candidate is tied with whoever came before
		// him (or #2 if he's #1). We also check if both the candidate
		// behind him and ahead of him has different scores - if that's
		// the case, then he's no longer tied and so we can exclude him
		// from the process.

		int lincount = 0, last_shown = 0;
		
		// For #1, compare against #2. We can't do this by reference,
		// otherwise, #2 could think it was different from #1 simply
		// because #1 was changed first. BLUESKY: Do better?
		pair<double, int> o_last = *(++l_ordering.begin());

		for (opos = l_ordering.begin(); source.ordinal_changed() && 
				opos != l_ordering.end(); ++opos) {
			if (!comparison.equals(o_last, *opos)) {
				last_shown = lincount;
				// Check if bracketed by other values.
				// If so, we're unique and need no longer
				// be part. Make prettier later.
				obpos = opos;
				++obpos;
				if (source.is_hopeful(opos->second) && 
						(obpos == l_ordering.end() || 
							!comparison.equals(*opos
								, *obpos))) {
					if (debug)
						cout << "Tie for " << 
							opos->second 
							<< " broken." << endl;
					source.exclude(opos->second);
				}
			}

			o_last = *opos;
			opos->first = last_shown;
			++lincount;
		}

		// If no ties remain, get out!
		if (source.get_num_hopefuls() < 2) continue; 

		// ------------------------ //

		// We're done for this iteration, so update the scores until
		// some of their ordinal relations change or we can no longer
		// update. If the user said no tiebreak, then don't update
		// at all.
		do
			updated = source.update(debug);
		while (!source.ordinal_changed() && updated && tiebreak);

		// If it's winner only and the winner is clear, we're done.
		if (winner_only && !source.is_hopeful(l_ordering.rbegin()->
					second)) {
			if (debug) cout << "Broke early." << endl;
			broke_early = true;
		}

		// repeat.
		if (debug)
			cout << endl;
	}

	// Transform the "ordering" into a true ordering.

	ordering out;

	for (opos = l_ordering.begin(); opos != l_ordering.end(); ++opos) {

		double eff_voters = source.lget_eff_voters(opos->second);

		if (debug)
			cout << "Candidate " << opos->second << " has score " 
				<< opos->first << " supported by " 
				<< eff_voters << " effective v." << endl;

		// Use the number of effective voters as a final tiebreak. We
		// add 2 so that the maximum alteration is definitely less than
		// 1.
		// *DO NOT* trust rankings beyond top if winner_only is true,
		// even if there seems to be a complete ranking.

		// TODO: Consider removing this since it's essentially a
		// combination with Approval and so may mess with Yee diagrams.
		out.insert(candscore(opos->second, opos->first - 
					(1.0 / (eff_voters + 2.0))));
	}

	if (debug) cout << iter << " iterations." << endl << endl;

	return(pair<ordering, bool>(out, broke_early));
}

pair<ordering, bool> vi_median_ratings::elect_inner(
		const list<ballot_group> & papers,const vector<bool> & hopefuls,
		int num_candidates, cache_map * /*cache*/, bool winner_only) const {

	// First, get the ballots into a more managable format.
	grad_fracile grad_struct = aggregate_ratings(papers, num_candidates, 
			hopefuls, normalize, 0, med_maximum);

	// Then get the ordering.
	return(get_ranks(grad_struct, 0.5, use_tiebreak, winner_only, false));
}

string vi_median_ratings::determine_name() const {
	string base = "Var.Median-" + dtos(med_maximum) + "(";
	if (normalize) {
		base += "norm";

		if (use_tiebreak)
			base += ", ";
	}
	if (use_tiebreak)
		base += "tiebreak";

	return (base + ")");
}
