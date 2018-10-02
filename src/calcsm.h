#ifndef CALCSM_H
#define CALCSM_H

#define PACKAGE "calcsm"
#define VERSION "0.1"

/** define qualquer instancia */
#define INSTANCE_ANY 0

#define TRUE 1
#define FALSE 0

extern int reallocate;
extern int debug;
extern int sort_method;
extern int gantt;

/* Definições dos parâmetros das instâncias */
extern char *inst_brief[];
extern char *inst_descriptions[];
extern int inst_num_params;

/** Abriga lista de instâncias.
 *
 * Contem todas as funções objetivo.
 */
typedef struct instancias {
	int inst;   /** Número da instância. */
	int nordens; /** Número de ordens nesta instância. */

	int Cmax;	/** Makespan.
	              * The makespan, defined as max(C1 , . . . , Cn ), is
	              * equivalent to the completion time of the last job to leave
	              * the system. A minimum makespan usually implies a good
	              * utilization of the machine(s).
	              */

    int sum_C;  /** Flow time. */
	int sum_wC;	/** Total weighted completion time.
	              * The sum of the weighted completion times of the n jobs gives
	              * an indication of the total holding or inventory costs
	              * incurred by the schedule. The sum of the completion times is
	              * in the literature often referred to as the flow time. The
	              * total weighted completion time is then referred to as the
	              * weighted flow time.*/
    int sum_wCmR; /** Discounted flow time.
                    * Definida como SUM w*(C_j - R_j) onde R é o release time.*/
   	int sum_U;	/** Total number of tardy jobs. */
	int sum_wU;	/** Weighted number of tardy jobs. */
   	int sum_T;	/** Total tardiness. */
	int sum_wT;	/** Total weighted tardiness.
	              * This is also a more general cost function than the total
	              * weighted completion time. */
    int Tmax;   /** Maximum tardiness. */
	int Lmax;	/** Maximum lateness.
                  * The maximum lateness, Lmax , is defined as
                  * max(L1 , . . . , Ln ). It measures the worst violation of
                  * the due dates. */
    int sum_wTpwE; /** Total weighted earliness and tardiness. */
                                    
    /* objetivos abaixo não foram originalmente solicitados. */
	int sum_E;	/** total earliness */
	int sum_wE;	/** total weighted earliness */
	int Emax;   /** Maximum earliness. */

	struct ordens *ohead;   /** link com instancia */
	
	struct instancias *next;    /** proxima instancia */
	struct instancias *prev;    /** instancia anterior */

} instancias;

int *inst_paramptr(instancias *i, int param);
int inst_paramval(instancias *i, int param);


void calc_inst(instancias *insthead);
void print_help(int rcode);
void iterate(instancias *instptr);

#endif
