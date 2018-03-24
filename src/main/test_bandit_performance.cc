
#include "../bandit/bandit.h"
#include "../bandit/lucb.h"

#include "../bandit/tests/bernoulli.h"

int main(int argc, const char ** argv) {

    int how_many = 4e6, i;

    std::vector<Bandit> bernoullis;
    for (i = 0; i < how_many; ++i) {
        bernoullis.push_back(new Bernoulli(drand48()));
    }

    int maxiter = 20;

    LUCB bandit_tester;

    bandit_tester.pull_bandit_arms(bernoullis, maxiter, false);
}
