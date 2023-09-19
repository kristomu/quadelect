// Dimensionality reduction for voting methods.

// For each election, produce an n^2 binary array which denotes whether candidate X was
// ranked ahead of candidate Y in the social order. If they're equal both the element
// corresponding to X>Y and Y>X are 0, otherwise one is 1 and the other is -1.

// This is used for dimensionality reduction in Python, to compare with Kevin Venzke's
// results.

#include "../tools/ballot_tools.h"
#include "../ballots.h"
#include "../tools/tools.h"

#include "../generator/all.h"
#include "../singlewinner/all.h"

#include "../random/random.h"

#include "../tools/ballot_tools.h"

#include <iostream>
#include <memory>

std::vector<int> get_pairwise_results(const ordering & social_order,
	int numcands) {
	std::vector<double> scores(numcands, 0);

	for (candscore x: social_order) {
		scores[x.get_candidate_num()] = x.get_score();
	}

	std::vector<int> pairwise_results;

	for (int i = 0; i < numcands; ++i) {
		for (int j = 0; j < numcands; ++j) {
			if (i == j) {
				continue;
			}

			if (scores[i] == scores[j]) {
				pairwise_results.push_back(0);
			}
			if (scores[i] > scores[j]) {
				pairwise_results.push_back(1);
			}
			if (scores[i] < scores[j]) {
				pairwise_results.push_back(-1);
			}
		}
	}

	return pairwise_results;
}

int main() {
	rng randomizer(10);
	iac impart_anon_c(true, false);

	std::vector<std::unique_ptr<election_method> > methods;

	methods.push_back(std::make_unique<ext_minmax>(CM_WV, false));
	methods.push_back(std::make_unique<ord_minmax>(CM_WV));
	methods.push_back(std::make_unique<copeland>(CM_WV));
	methods.push_back(std::make_unique<keener>(CM_WV, 0.001, true, false));
	methods.push_back(std::make_unique<sinkhorn>(CM_WV, 0.001, true));
	methods.push_back(std::make_unique<keener>(CM_PAIRWISE_OPP, 0.001, true,
			false));
	methods.push_back(std::make_unique<sinkhorn>(CM_PAIRWISE_OPP, 0.001,
			true));
	methods.push_back(std::make_unique<borda>(PT_WHOLE));
	methods.push_back(std::make_unique<plurality>(PT_WHOLE));
	methods.push_back(std::make_unique<ext_plurality>(PT_WHOLE));
	methods.push_back(std::make_unique<antiplurality>(PT_WHOLE));
	methods.push_back(std::make_unique<dsc>());

	// Something better than this would be a good idea, but I need to turn
	// the unique pointers into shared ones or something first...
	std::shared_ptr<ext_plurality> hack_eplur =
		std::make_shared<ext_plurality>(PT_WHOLE);

	methods.push_back(std::make_unique<loser_elimination>(
			std::make_shared<ext_plurality>(PT_WHOLE), false, true));
	methods.push_back(std::make_unique<loser_elimination>(
			std::make_shared<borda>(PT_WHOLE), false, true));
	methods.push_back(std::make_unique<loser_elimination>(
			std::make_shared<antiplurality>(PT_WHOLE), false, true));
	methods.push_back(std::make_unique<loser_elimination>(
			std::make_shared<dsc>(), false,	true));
	methods.push_back(std::make_unique<loser_elimination>(
			hack_eplur, true, true));
	methods.push_back(std::make_unique<comma>(
			std::make_shared<smith_set>(), hack_eplur));
	methods.push_back(std::make_unique<comma>(
			std::make_shared<smith_set>(),
			std::make_shared<loser_elimination>(hack_eplur, true, true)));

	int iterations = 10000;
	int numvoters = 23, numcands = 4;

	std::vector<std::vector<int> > pw_results(methods.size());
	cache_map cache;

	size_t method_idx;

	for (int i = 0; i < iterations; ++i) {
		cache.clear();

		election_t ballots = impart_anon_c.generate_ballots(
				numvoters, numcands, randomizer);

		for (method_idx = 0; method_idx < methods.size(); ++method_idx) {

			ordering outcome = methods[method_idx]->elect(ballots, numcands, &cache,
					false);

			std::vector<int> outcome_pw = get_pairwise_results(outcome, numcands);
			pw_results[method_idx].insert(pw_results[method_idx].end(),
				outcome_pw.begin(), outcome_pw.end());
		}
	}

	for (method_idx = 0; method_idx < methods.size(); ++method_idx) {
		std::cout << methods[method_idx]->name() << ";";
		std::copy(pw_results[method_idx].begin(), pw_results[method_idx].end(),
			std::ostream_iterator<int>(std::cout, " "));
		std::cout << std::endl;
	}

	return 0;
}