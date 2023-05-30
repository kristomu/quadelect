
#include "fpa_experiment.h"
#include "../method.h"
#include "../positional/simple_methods.h"

#include <iterator>
#include <vector>

std::pair<ordering, bool> fpa_experiment::elect_inner(
	const std::list<ballot_group> & papers,
	const std::vector<bool> & hopefuls,
	int num_candidates, cache_map * cache,
	bool winner_only) const {

	condmat condorcet_matrix = condmat(papers, num_candidates,
			CM_PAIRWISE_OPP);

	plurality plur(PT_WHOLE);

	// ???
	// Why doesn't inheritance work here???
	ordering plurality_result = ((election_method *)&plur)->elect(papers,
			hopefuls, num_candidates, NULL, false);

	/*ordering elect(const std::list<ballot_group> & papers,
	            const std::vector<bool> & hopefuls,
	            int num_candidates, cache_map * cache,
	            bool winner_only) const;*/


	std::vector<double> plur_result_table(num_candidates, 0);

	size_t i, j;
	double numvoters = 0;

	for (ordering::const_iterator pos = plurality_result.begin();
		pos != plurality_result.end(); ++pos) {

		plur_result_table[pos->get_candidate_num()] = pos->get_score();
		numvoters += pos->get_score();
	}

	// For each candidate X, the score is for each other candidate Y
	// numvoters if X beats Y, otherwise fpX - fpY.

	std::vector<std::pair<std::list<double>, size_t> > score_lists;

	for (i = 0; i < condorcet_matrix.get_num_candidates(); ++i) {
		if (!hopefuls[i]) {
			continue;
		}
		std::pair<std::list<double>, size_t> next_score_element(
			std::list<double>(), i);

		// Forest Simmons's suggestion: number of ballots on which the
		// highest ranked candidate is someone who is beaten pairwise by X
		// minus the number of ballots on which the first place candidate is
		// someone who beats X pairwise, plus twice the number of ballots
		// where X is the highest ranked candidate.

		double A_first = plur_result_table[i];
		double other_score_tally = 0;

		for (j = 0; j < condorcet_matrix.get_num_candidates(); ++j) {
			if (!hopefuls[j] || j == i) {
				continue;
			}

			double AoverX = condorcet_matrix.get_magnitude(i, j, hopefuls);
			double XoverA = condorcet_matrix.get_magnitude(j, i, hopefuls);

			if (AoverX > XoverA) {
				other_score_tally += plur_result_table[j];
			} else if (XoverA > AoverX) {
				other_score_tally -= plur_result_table[j];
			}
		}
		next_score_element.first.push_back(2 * A_first + other_score_tally);

		// Earlier idea below
		/*for (j = 0; j < condorcet_matrix.get_num_candidates(); ++j) {
		    if (!hopefuls[j]) continue;
		    double AoverX = condorcet_matrix.get_magnitude(i, j, hopefuls);
		    double XoverA = condorcet_matrix.get_magnitude(j, i, hopefuls);

		    if (AoverX > XoverA) {
		        next_score_element.first.push_back(AoverX);
		    } else {
		        next_score_element.first.push_back(plur_result_table[i]-plur_result_table[j]);
		    }
		}*/
		next_score_element.first.sort();
		score_lists.push_back(next_score_element);
	}

	std::sort(score_lists.begin(), score_lists.end());

	ordering out;
	int score = 0;

	for (i = 0; i < score_lists.size(); ++i) {
		if (!hopefuls[score_lists[i].second]) {
			continue;
		}
		if (i > 0 && score_lists[i].first > score_lists[i-1].first) {
			++score;
		}
		out.insert(candscore(score_lists[i].second, score));
	}

	return (std::pair<ordering, bool>(out, false));
}
