
#include "../bandit/bandit.h"
#include "../bandit/lilucb.h"

#include "../bandit/tests/bernoulli.h"

int main(int argc, const char ** argv) {

	int how_many = 4e6, i;

	std::vector<Bandit> bernoullis;
	for (i = 0; i < how_many; ++i) {
		bernoullis.push_back(new Bernoulli(drand48()));
	}

	int maxiter = 20;

	Lil_UCB bandit_tester;
	bandit_tester.load_bandits(bernoullis);

	bandit_tester.pull_bandit_arms(maxiter);
}
