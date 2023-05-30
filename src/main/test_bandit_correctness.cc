
#include "../bandit/bandit.h"
#include "../bandit/lilucb.h"

#include "../bandit/tests/bernoulli.h"

#include <iostream>

int main(int argc, const char ** argv) {

	int how_many = 500, i;

	std::vector<Bandit> bernoullis;
	double maxmean = 0;

	for (i = 0; i < how_many; ++i) {
		double this_arm_mean = 0.005 * drand48() + i/(double)(2*how_many);
		bernoullis.push_back(new Bernoulli(this_arm_mean));
		maxmean = std::max(maxmean, this_arm_mean);
	}

	random_shuffle(bernoullis.begin(), bernoullis.end());

	int maxpulls = 40000, iters_so_far = 0;

	Lil_UCB bandit_tester;
	bandit_tester.load_bandits(bernoullis);

	double progress;

	while ((progress = bandit_tester.pull_bandit_arms(bernoullis,
					maxpulls)) != 1) {
		std::cout << "Not done yet (progress is " << progress << ")" << std::endl;
		const Bandit * best = bandit_tester.get_best_bandit_so_far();
		std::cout << "Best so far is " << best->name() << std::endl;
		std::cout << "Actual maximum is " << maxmean << std::endl;
		iters_so_far += maxpulls; // or thereabouts
	}

	std::cout << "It took about " << iters_so_far
		<< " pulls to find that out." << std::endl;
}
