
#include "../bandit/lilucb.h"
#include "../simulator/stubs/bernoulli.h"

// one of the few times using native random might be OK
#include "../hack/msvc_random.h"

#include <iostream>

int main(int argc, const char ** argv) {

	int how_many = 500, i;
	bool maximize = true;

	std::vector<std::shared_ptr<simulator> > bernoullis;
	double maxmean = 0, minmean = 1;

	for (i = 0; i < how_many; ++i) {
		double this_arm_mean = 0.001 * drand48() + i/(double)(2*how_many);
		bernoullis.push_back(std::make_shared<bernoulli_stub>(this_arm_mean,
				maximize, 0));
		maxmean = std::max(maxmean, this_arm_mean);
		minmean = std::min(minmean, this_arm_mean);
	}

	random_shuffle(bernoullis.begin(), bernoullis.end());

	double seconds_per_round = 0.5;

	Lil_UCB bandit_tester;
	bandit_tester.load_arms(bernoullis);

	double progress;

	while ((progress = bandit_tester.timed_pull_bandit_arms(
					seconds_per_round)) != 1) {
		std::cout << "Not done yet (progress is " << progress << ")" << std::endl;
		std::shared_ptr<simulator> best = bandit_tester.get_best_arm_so_far();
		std::cout << "Best so far is " << best->name() << std::endl;
		if (maximize) {
			std::cout << "Actual maximum is " << maxmean << std::endl;
		} else {
			std::cout << "Actual minimum is " << minmean << std::endl;
		}
	}

	std::shared_ptr<simulator> best = bandit_tester.get_best_arm_so_far();
	std::cout << "Bandit says the best is " << best->name() << std::endl;

	std::cout << "It took " << bandit_tester.get_total_num_pulls()
		<< " pulls to find that out." << std::endl;
}
