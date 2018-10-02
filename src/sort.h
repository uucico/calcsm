#ifndef SORT_H
#define SORT_H

#include <string.h>

#include "calcsm.h"
#include "orders.h"
#include "utlist.h"

/** obtem numero randomico inteiro de 0 a i-1 (i posicoes) */
#define rnd(i) ((rand()%i))

/* "rand", "seq", "expet", "annl", "edd"  */
#define SORT_NOMETHOD -1 /* usado para erros na linha de comando */
#define SORT_RANDOM 0
#define SORT_SEQ 1
#define SORT_W_LBE_LBT 2
#define SORT_ANNL 3
#define SORT_EDD 4
#define SORT_ENUM 5
#define SORT_TWKR_BY_TIS 6
#define SORT_ORDER 10
#define SORT_STARTT 11

extern char *sort_methods[];
extern char *sort_descriptions[];
extern int sort_num_methods;
extern int sort_number;


int switch_sort(char *sptr);
int ordemstart(ordens *a, ordens *b);
int ordemcmp(ordens *a, ordens *b);
int ordemseq(ordens *a, ordens *b);
int ordemedd(ordens *a, ordens *b);

void sort_inst(instancias *insthead, int type);

#endif
