// c antilamer

#include <vector>
using namespace std;

long double dblchoose(int n, int k);
unsigned long long choose(int n, int k); // choose op
int num(vector<bool>::iterator it, int n,int k); // get number of a permutation
vector<bool> perm(int permN, int n,int k); // get permutation by number
