#include "odm_gen.h"

#include <iostream>
#include <stdexcept>

// Generic class for methods similar to the Offense-Defense model of Govan et
// al. These methods are Sinkhorn-esque: they involve determining row and column
// factors depending on each other and on either the rows or columns of the
// pairwise matrix. The calculation of the row and column factors is alternated,
// and the score ultimately converges.

std::pair<ordering, bool> odm_gen::pair_elect(const abstract_condmat &
	input,
	const std::vector<bool> & hopefuls, cache_map * cache,
	bool winner_only) const {

	bool debug = false;

	// For now, don't run this on data with values < 0.
	// TODO: make the "lifting constant" (1e-6) configurable. It is added
	// so that the calculation will converge.

	double eps = 1e-6;
	int maxiter = 500;

	double convergence = 0;
	size_t counter, sec;
	int iter = 0;

	// Like Sinkhorn and Keener, we'll be using the matrix a lot of times,
	// so copy it over. Sic transit mathematical elegance.

	std::vector<int> permitted_candidates;
	permitted_candidates.reserve(input.get_num_candidates());

	for (counter = 0; counter < hopefuls.size(); ++counter)
		if (hopefuls[counter]) {
			permitted_candidates.push_back(counter);
		}

	size_t num_hopefuls = permitted_candidates.size();
	std::vector<double> offense(num_hopefuls, eps), defense(num_hopefuls, eps);
	std::vector<double> score(num_hopefuls, INFINITY), old_score;

	std::vector<std::vector<double> > matrix(num_hopefuls, std::vector<double>(
			num_hopefuls, 0));

	for (counter = 0; counter < num_hopefuls; ++counter)
		for (sec = 0; sec < num_hopefuls; ++sec) {
			if (counter == sec) {
				continue;
			}

			double curval = input.get_magnitude(
					permitted_candidates[counter],
					permitted_candidates[sec]);

			// If there are any negative values, there's nothing
			// we can do, so abort early.
			// Hack for now: return an all-empty ballot. "I refuse
			// to make that call."
			if (curval < 0) {
				std::pair<ordering, bool> retval;
				for (counter = 0; counter < num_hopefuls;
					++counter)
					retval.first.insert(candscore(
							permitted_candidates[counter],
							0));
				return (retval);
			}

			matrix[counter][sec] = curval;
		}

	do {
		if (debug) {
			std::cout << name() << " at iter " << iter << std::endl;
		}

		old_score = score;

		// A candidate's offense rating is equal to the sum over all
		// other candidates, of the victory against that candidate
		// times one divided by the candidate's defense rating.

		fill(offense.begin(), offense.end(), 0);

		for (counter = 0; counter < num_hopefuls; ++counter) {
			for (sec = 0; sec < num_hopefuls; ++sec)
				if (counter != sec)
					offense[counter] += nltrans(eps +
							matrix[counter][sec],
							defense[sec]);

			if (debug)
				std::cout << permitted_candidates[counter] << ": " <<
					"offense is " << offense[counter]
					<< std::endl;
		}

		// Renorm if required.
		ir_norm(offense);

		// A candidate's defense rating is equal to the sum over all
		// other candidates, of that candidate's victory over him,
		// times one divided by the candidate's offense rating.

		fill(defense.begin(), defense.end(), 0);

		for (counter = 0; counter < num_hopefuls; ++counter) {
			for (sec = 0; sec < num_hopefuls; ++sec)
				if (hopefuls[sec] && counter != sec)
					defense[counter] += nltrans(eps +
							matrix[sec][counter],
							offense[sec]);

			if (debug)
				std::cout << permitted_candidates[counter] << ": " <<
					"defense is " << defense[counter]
					<< std::endl;
		}

		// Again, renorm if required.
		ir_norm(defense);

		// Now determine scores so we can check how far we've
		// converged.

		for (counter = 0; counter < matrix.size(); ++counter) {
			score[counter] = get_score(offense[counter],
					defense[counter]);

			if (isnan(score[counter])) {
				throw std::runtime_error("ODM: Failed to converge!");
			}

			if (debug)
				std::cout << "Score for " <<
					permitted_candidates[counter]
					<< " is " << score[counter] << std::endl;
		}

		// And then check that.
		// Don't do it if we're on the first iteration, because then
		// old_score hasn't been set yet.

		if (iter == 0) {
			++iter;
			continue;
		}

		convergence = 0;

		for (counter = 0; counter < matrix.size(); ++counter)
			convergence += (score[counter] - old_score[counter]) *
				(score[counter] - old_score[counter]);

		// If it's not converging, do the most of it and get outta
		// here.
		std::cout << "Convergence " << convergence << std::endl;
		if (!isfinite(convergence)) {
			if (debug) // error
				std::cout << "ERROR: Failed to converge, bailing."
					<< std::endl;

			score = old_score;
			convergence = -1;
		}

		++iter;

		if (debug) {
			std::cout << "Convergence is " << convergence << std::endl;
		}

	} while (convergence > tolerance && iter < maxiter);

	// Determine final scores and renormalize so the sum is 1.
	// Only do this if there're no infinities. If there are
	// infinities, only sum the finite components, and set the
	// infinities to 10, which will exceed any finite value due
	// to the renormalization.

	// This is somewhat of a hack; the natural interpretation of any
	// infinities is that the candidate/s with infinite score beats
	// anybody else by any amount you can name, and that the (finite)
	// differences of the non-infinite score candidates should only
	// be used to determine which of the finite candidates are best.
	// We can't really capture that here, due to the scores having to
	// be finite.
	double sum = 0;
	for (counter = 0; counter < score.size(); ++counter) {
		if (!isfinite(score[counter])) {
			continue;
		}
		sum += score[counter];
	}

	for (counter = 0; counter < score.size(); ++counter) {
		if (isfinite(score[counter])) {
			score[counter] /= sum;
		} else {
			score[counter] = copysign(10, score[counter]);
		}

		if (debug)
			std::cout << "Final score for " <<
				permitted_candidates[counter] << ": "
				<< score[counter] << std::endl;
	}

	// All done, return the results.
	return (std::pair<ordering, bool>(ordering_tools().
				indirect_vector_to_ordering(score,
					permitted_candidates), false));
}

std::string odm_gen::pw_name() const {

	std::string ret = odm_name() + "(" + dtos(tolerance) + ")";

	return (ret);
}

