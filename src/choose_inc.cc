// c antilamer

#include <vector>
using namespace std;

unsigned long long fact(int n) {
    unsigned long long res=1;
    while(n>0) res*=n--;
    return res;
}

// Need to use some other method of finding C because factorials get real
// large real quick.

/*unsigned long long choose(int n,int k) {
    if(n<k) return 0;
    return fact(n)/(fact(k)*fact(n-k));
}*/

#include <iostream>

// Wikipedia implementation of n choose k
long double dblchoose(int n, int k) {
	if (k > n) return(0);

	if (k > n/2)
		k = n-k;

	long double accum = 1;

	for (int i = 1; i <= k; ++i)
		accum = accum * (n-k+i) / i;

	return(accum);
}

unsigned long long choose(int n, int k) {

	long double accum = dblchoose(n, k);

	//std::cout << " Hello: " << accum << endl;

	// This signals "too many to count
	if (accum > (unsigned long long)((long long)(-1)))
		return(-1);

	return(accum + 0.5);
}

// Get number of a permutation starting at 'it'
int num(vector<bool>::iterator it, int n,int k) {
    if(n<=0) return 0;
    if(k<=0) return 0;
    if(*it) return num(it+1, n-1, k-1);
    return choose(n-1,k-1)+num(it+1, n-1,k);
}

// Get a permutation by number
vector<bool> perm(unsigned int permN, int n,int k) {
    vector<bool> res, temp;
    if (k==0) res.resize(res.size()+n, false);
    if ((n==0) || (k==0)) return(res);

    if(permN<choose(n-1,k-1)) {
        res.push_back(true);
        temp=perm(permN,n-1,k-1);
    } else {
        res.push_back(false);
        temp=perm(permN-choose(n-1,k-1), n-1,k);
    }
    res.insert(res.end(), temp.begin(), temp.end());
    return res;
}
