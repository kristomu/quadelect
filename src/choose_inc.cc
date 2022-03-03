// c antilamer

#include <limits>
#include <vector>
#include <iostream>
#include <stdexcept>

#include "tools/tools.h"

unsigned long long fact(int n) {
    unsigned long long res=1;
    while(n>0) res*=n--;
    return res;
}

// Wikipedia implementation of n choose k
// Now uses recursive version, though perhaps somewhat overkill...

unsigned int choose(unsigned int n, unsigned int k) {
    if (k > n) 
        return(0);
    
    if (k > n-k)
        k = n-k;
    
    if (k == 0 || n <= 1)
        return (1);

    long double first_half = choose(n-1, k),
                second_half = choose(n-1, k-1);

    if (first_half > std::numeric_limits<int>::max() - second_half) {
        throw std::overflow_error("dblchoose: overflow n="+itos(n) + " k=" + 
            itos(k));
    }

    return(first_half + second_half);
}

// Get number of a permutation starting at 'it'
unsigned int num(std::vector<bool>::iterator it, int n,int k) {
    if(n<=0) return 0;
    if(k<=0) return 0;
    if(*it) return num(it+1, n-1, k-1);
    return choose(n-1,k-1)+num(it+1, n-1,k);
}

// Get a permutation by number
std::vector<bool> perm(unsigned int permN, int n,int k) {
    std::vector<bool> res, temp;
    
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
