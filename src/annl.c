#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <gsl/gsl_siman.h>
#include <gsl/gsl_ieee_utils.h>
#include <gsl/gsl_rng.h>


#include "orders.h"
#include "calcsm.h"
#include "sort.h"
#include "utlist.h"

#include "annl.h"

int num;
int *oldseq;
int annl_optobj;
ordens **annl_ovec;
instancias *annl_instptr;

/* calcula energia */
double calcula_energia(void *xp) {
    int *x = (int *)xp;
    int i;
    double obj;

    for (i=0; i < num; i++)
        (annl_ovec[i])->seq = x[i];
  
    sort_inst(annl_instptr,SORT_SEQ);
    calc_inst(annl_instptr);

    obj = inst_paramval(annl_instptr,annl_optobj);

    for (i=0; i < num; i++)
        (annl_ovec[i])->seq = oldseq[i];
  
    sort_inst(annl_instptr,SORT_SEQ);
    calc_inst(annl_instptr);
    
    memcpy(oldseq, x, (sizeof(int)*num));

    return obj;
    
}

/* calcula distancia */
double M1(void *xp, void *yp) {
    int *seq1 = (int *)(((xp_s*)xp)->sequencia);
    int *seq2 = (int *)(((xp_s*)yp)->sequencia);
    int i;
    double dist = 0;
    
    printf(",");

    printf("procurando dist ate %d\n", ((xp_s *)xp)->instptr->nordens);
    for (i = 0; i < ((xp_s *)xp)->instptr->nordens; i++) {
        dist += ((seq1[i] == seq2[i]) ? 0 : 1);
    }
    printf("dist = %lf\n", dist);
    
    return dist;
}
      
void S1(const gsl_rng * r, void *xp, double step_size) {
    int *x = (int *)xp;
    int x1, x2, tmp;
    
    step_size = 0;
    
    x1 = (gsl_rng_get (r) % (num));
    do {
       x2 = (gsl_rng_get (r) % (num));
    } while (x2 == x1);
    
    tmp = x[x1];
    x[x1] = x[x2];
    x[x2] = tmp;

}
     
void P1(void *xp) {
    ordens *ordemptr;
    int *x = (int *)xp;
    int i;
    
    printf("  [ Seq: ");
    
    for (i = 0; i < num; i++) {
        printf("%2d ", x[i]);
    }
    printf("]");
}

void solve(instancias  *instptr, int optobj) {
    ordens *ordemptr;
    int i;
    gsl_rng * r = gsl_rng_alloc (gsl_rng_env_setup()) ;
    int *inicial;
    gsl_siman_params_t params = {N_TRIES, ITERS_FIXED_T, STEP_SIZE, 
        K, T_INITIAL, MU_T, T_MIN};

    gsl_ieee_env_setup ();

    num = instptr->nordens;
    /* aloca sequencia inicial */     
    inicial = (int *)malloc(sizeof(int)*num);
    oldseq = (int *)malloc(sizeof(int)*num);    
    for (i=1; i <= instptr->nordens; i++) {
        inicial[i-1] = i;
        oldseq[i-1] = i;
    }
    
    annl_optobj = optobj;
    annl_instptr = instptr; 
    annl_ovec = (ordens**)malloc(num*sizeof(ordens*));
    i = 0;
        DL_FOREACH(instptr->ohead, ordemptr)
            annl_ovec[i++] = ordemptr;

    if (debug)
        gsl_siman_solve(r, inicial, calcula_energia, S1, NULL, P1, NULL, NULL, NULL, sizeof(int)*num, params);
    else
        gsl_siman_solve(r, inicial, calcula_energia, S1, NULL, NULL, NULL, NULL, NULL, sizeof(int)*num, params);
 
    for (i=0; i < num; i++)
        (annl_ovec[i])->seq = inicial[i];
  
    sort_inst(annl_instptr,SORT_SEQ);
    calc_inst(annl_instptr);
                 
    gsl_rng_free (r);
    
    free(annl_ovec);
    free(inicial);
    free(oldseq);

}
