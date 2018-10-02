#ifndef PERMUTATION_H
#define PERMUTATION_H

#ifdef __cplusplus /* If this is a C++ compiler, use C linkage */
extern "C" {
#endif


int prev_perm(int *myints, int n);

int next_perm(int *myints, int n);

#ifdef __cplusplus /* If this is a C++ compiler, end C linkage */
}
#endif


#endif
