// prev_permutation and next permutation
#include <iostream>
#include <algorithm>

#include "permutation.hpp"

using namespace std;

int prev_perm(int *myints, int n) {

  return prev_permutation (myints,myints+n);

}

int next_perm(int *myints, int n) {

  return next_permutation(myints, myints+n);
}
