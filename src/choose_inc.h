#include <vector>

unsigned int choose(unsigned int n, unsigned int k); // choose op
unsigned int num(vector<bool>::iterator it, int n,
	int k); // get number of a permutation
std::vector<bool> perm(int permN, int n,
	int k); // get permutation by number