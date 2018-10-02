#ifndef ANNL_H
#define ANNL_H
#include <gsl/gsl_siman.h>

#include "orders.h"
#include "calcsm.h"

typedef struct xp_s {
    instancias *instptr;
    int *sequencia;
    ordens **ovec;
    int perm_num;
    int optobj;
} xp_s;
     
#define N_TRIES 10             /* how many points do we try before stepping */
#define ITERS_FIXED_T 1     /* how many iterations for each T? */
#define STEP_SIZE 1.0           /* max step size in random walk */
#define K 1.0                   /* Boltzmann constant */
#define T_INITIAL 5000.0        /* initial temperature */
#define MU_T 1.002              /* damping factor for temperature */
#define T_MIN 5.0e-1

double calcula_energia(void *xp);

double M1(void *xp, void *yp);
     
void S1(const gsl_rng * r, void *xp, double step_size);
     
void P1(void *xp);

void solve(instancias  *instptr, int optobj);

#endif
