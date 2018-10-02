#ifndef ORDERS_H
#define ORDERS_H

/** \defgroup ordens Ordens
    Ordens são entidades...
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "calcsm.h"
#include "utlist.h"

/** Estrutura de dados para armazenar informações sobre uma ordem.
  * \ingroup ordens
  */
typedef struct ordens {

	int inst;	/** instancia */
	int ordem;	/** ordem */
	int r;		/** release date */
	int d;		/** due date */
	int w;		/** weight */
	int wT;		/** weighted tardiness */
	int wE;		/** weighted earliness */
	int p;		/** processing time */
	float z;      /** dispatch rule ranking */

	/* parametros do roteiro */
	int seq;	/** sequencia */
	int startt; /** start time */
	int realloc; /** realocada por restricao? */

	/** link com as instancias */
	struct instancias *instptr;

	/* DL */
	struct ordens *next;    /** proxima ordem */
	struct ordens *prev;    /** ordem anterior */
} ordens;

/** Carrega ordens do arquivo fp.
 * \ingroup ordens
 *
 * \param fp file descriptor para arquivo a ser lido.
 * \return ponteiro para lista de instâncias ou ponteiro nulo caso
 */
instancias *loadorders(FILE *fp);

/** Imprime ordens.
  * \ingroup ordens
  * \param insthead Poteiro para head da instância.
  * \param inst Número da instância.
  */
void dump_ordens(instancias *insthead, int inst);

void dump_ordens_gnuplot(FILE *fd, instancias *insthead, int inst);

void commit_order(instancias *insthead, ordens *optr);

int check_constraints(instancias *insthead, ordens *optr);

void insert_after(ordens *a, ordens *b);

void swap_orders(ordens *a, ordens *b);

#endif
