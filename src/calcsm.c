#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "utlist.h"

#include "calcsm.h"
#include "orders.h"
#include "sort.h"
#include "permutation.hpp"
#include "annl.h"

int reallocate=FALSE;
int optobj=9;
int debug=FALSE;
int gantt=FALSE;
int sort_method = SORT_RANDOM;

/* Definições dos parâmetros das instâncias */
char *inst_brief[] = {"inst",
                    "nordens",
                    "Cmax",
                    "sum_C",
                    "sum_wC",
                    "sum_wCmR",
                    "sum_U",
                    "sum_wU",
                    "sum_T",
                    "sum_wT",
                    "Tmax",
                    "Lmax",
                    "sum_wTpwE",
                    "sum_E",
                    "sum_wE",
                    "Emax"};
char *inst_descriptions[] = { "Instancia",
                            "Total de ordens",
                            "Makespan",
                            "Tempo de fluxo",
                            "Fluxo ponderado",
                            "Fluxo desc. p.",
                            "Ord. atrasadas",
                            "Ord. atras. pond",
                            "Atraso total",
                            "Atras. pond. tot",
                            "Atraso maximo",
                            "Lateness maximo",
                            "Atras+adiant pond",
                            "Adiant. total",
                            "Adiant. ponderado",
                            "Adiant. maximo"};
int inst_num_params = 16;

int *inst_paramptr(instancias *i, int param) {
    switch (param) {
        case 0: return &(i)->inst;
        case 1: return &(i)->nordens;
        case 2: return &(i)->Cmax;
        case 3: return &(i)->sum_C;
        case 4: return &(i)->sum_wC;
        case 5: return &(i)->sum_wCmR;
        case 6: return &(i)->sum_U;
        case 7: return &(i)->sum_wU;
        case 8: return &(i)->sum_T;
        case 9: return &(i)->sum_wT;
        case 10: return &(i)->Tmax;
        case 11: return &(i)->Lmax;
        case 12: return &(i)->sum_wTpwE;
        case 13: return &(i)->sum_E;
        case 14: return &(i)->sum_wE;
        case 15: return &(i)->Emax;
    };
    return NULL;
}

int inst_paramval(instancias *i, int param) {
    switch (param) {
        case 0: return (i)->inst;
        case 1: return (i)->nordens;
        case 2: return (i)->Cmax;
        case 3: return (i)->sum_C;
        case 4: return (i)->sum_wC;
        case 5: return (i)->sum_wCmR;
        case 6: return (i)->sum_U;
        case 7: return (i)->sum_wU;
        case 8: return (i)->sum_T;
        case 9: return (i)->sum_wT;
        case 10: return (i)->Tmax;
        case 11: return (i)->Lmax;
        case 12: return (i)->sum_wTpwE;
        case 13: return (i)->sum_E;
        case 14: return (i)->sum_wE;
        case 15: return (i)->Emax;
    };
    return 0;
}

void dump_instancias(instancias *head, int inst) {
	instancias *ptr;
    int i;

    printf("\n                 R E S U M O   D A   I N S T Â N C I A\n");

    for (i=0; i<2; i++) {
    printf("\n   %17s ",inst_descriptions[i]);
    DL_FOREACH(head,ptr)
        if (inst == INSTANCE_ANY || ptr->inst == inst)    
            printf("|%5d",inst_paramval(ptr,i));        
            
    }            
    printf("\n---------------------");
    DL_FOREACH(head,ptr)
        if (inst == INSTANCE_ANY || ptr->inst == inst)    
            printf("------");
    printf("\n");
    
    for (i=2; i<inst_num_params; i++) {
        printf("%2d %17s ",i-1,inst_descriptions[i]);


        DL_FOREACH(head,ptr) {
            if (inst == INSTANCE_ANY || ptr->inst == inst)
                printf("|%5d",inst_paramval(ptr,i));
        }
        
        printf("\n");
        
    }
}

/* atravessa sequencia da instancia e determina parametros da
 * sequencia, calculando parametros finais da instancia.
 */
void calc_inst(instancias *insthead) {
    ordens *ohead_pend = NULL;  /* estrutura para ordens nao alocadas */
    int i;
    ordens *optr, *optrtemp, *opendptr, *optrtemp2;
    
    for (i=2;i<inst_num_params;i++)
        *(inst_paramptr(insthead,i)) = 0;
    
    DL_FOREACH_SAFE(insthead->ohead, optr, optrtemp) {
        if (check_constraints(insthead, optr) == FALSE) {
        
            if (reallocate) {
    //            printf("ordem %d nao pode ser colocada em starrt = %d\n", optr->ordem, insthead->Cmax);
                DL_DELETE(insthead->ohead,optr);    /* remove da sequencia de ordens */
                DL_APPEND(ohead_pend,optr);         /* acrescenta na lista de pendencias */
                optr->realloc = TRUE;
                continue;
            }
            
  //          printf("atrasando ordem %d para startt = %d\n", optr->ordem, optr->r);
            insthead->Cmax = optr->r;
        }
             
//        printf("commitando ordem %d\n", optr->ordem);
        commit_order(insthead, optr);


/* aloca multiplas ordens */
        int sair = 0;
        do {
           if (ohead_pend == NULL)
                sair = 1;
            DL_FOREACH_SAFE(ohead_pend, opendptr, optrtemp2) {
                printf("Testando pendente %d r %d em starrt=%d: ", opendptr->ordem, opendptr->r, insthead->Cmax);
                if (opendptr->r < insthead->Cmax) {
                     printf("iserido!\n");
                    DL_DELETE(ohead_pend, opendptr);
                    insert_after(opendptr,optr);
                    commit_order(insthead, opendptr);
                    break;
                } else {
                    printf("impossivel next =.\n");
                    if (!opendptr->next)
                        sair = 1;
                }
            }

        } while (sair == 0);
    }

}

int main(int argc, char *argv[]) {
	int c,i,j;

	FILE *fp, *gfd;
	ordens *ordemptr;

	instancias *insthead = NULL;	/* head para lista */
	instancias *instptr;		    /* estrutura temp e ponteiro */

	/* inicia */
	int optinst = INSTANCE_ANY;     /* qual instancia a exibir */
	sort_method = SORT_SEQ;         /* default: sequencia imposta */
	fp = stdin;


    srand(getpid());            /* inicializa semente */
    for (i=0;i<1000;i++) {
        rand();
    }

	printf("%s %s: calcula sequenciamento de máquina única\n\n", PACKAGE, VERSION); 
	
	sort_number=1;
	opterr = 0;
	while ((c = getopt (argc, argv, "hvi:s:f:o:c:rp")) != -1)
		switch (c) {
			case 'h':
				print_help(0);
        	case 'r':
				reallocate=TRUE;
				break;
			case 'v':
				debug = TRUE;
				break;
			case 'i':
				optinst = atoi(optarg);
				break;
    		case 'o':
				optobj = atoi(optarg)+1;
				if (optobj < 1 || optobj > 14) {
				    fprintf(stderr, "%s: Objetivo inválido.\n", PACKAGE);
				    print_help(1);
			    }
				break;
			case 'c':
				sort_number = atoi(optarg);
				break;
			case 'p':
				gantt = TRUE;
				break;				
			case 'f':
				fp = fopen((const char *)optarg,"r");
				if (!fp) {
					fprintf(stderr, "%s: Erro abrindo arquivo de entrada %s: ",
					    PACKAGE, optarg);
					perror("");
					return 1;
				}
				break;
			case 's':
				/* obtem metodo de sequenciamento */
				sort_method = switch_sort(optarg);
				if (sort_method == SORT_NOMETHOD) {
					fprintf(stderr, "Metodo de sequenciamento inválido.\n");
					print_help(1);
				}
				break;
			case ':':
				fprintf(stderr, "%s: Opção -%c requer parâmetro.\n", PACKAGE, optopt);
				break;
				//FIXME
			case '?':
				if (optopt == 'i')
					fprintf(stderr, "Opcao -i requer argumento.\n");
				return 1;
			default:
				return 0;
	}

    insthead = loadorders(fp);

    if (!insthead) {
        perror("Nenhuma instancia carregada");
        return 1;
    }
    
    if (debug) printf("Iniciando com método: %s (%s)...\n\n", sort_descriptions[sort_method],
        sort_methods[sort_method]);
	/* SORTEIA INSTANCIAS */
    if (optinst == INSTANCE_ANY)
       	DL_FOREACH(insthead,instptr) {
       	    iterate(instptr);
       	}
    else {
        DL_SEARCH_SCALAR(insthead,instptr,inst,optinst);
        iterate(instptr);
    }

    /* imprime resultado */
    dump_ordens(insthead,optinst);
	dump_instancias(insthead,optinst);
	
	if (gantt) {
	    //FIXME: Apenas para validar a solução.
	    
	    gfd = fopen("/tmp/.gantt","w");
	    dump_ordens_gnuplot(gfd,insthead,optinst);
        fclose(gfd);
        system("python plot/gantt.py -t Programação -c plot/colors.cfg -o /tmp/.ganttgp /tmp/.gantt");
        system("gnuplot -persist /tmp/.ganttgp");
    
//        unlink("/tmp/.gantt");
//        unlink("/tmp/.ganttgp");
    }
	fclose(fp);
    return 0;
    
}


void iterate(instancias *instptr) {
	int *melhor_solucao, melhor_objetivo=0;
	ordens *ordemptr, **ovec;
	int *sequencia;
	int i,j;

	switch (sort_method) {
	    case SORT_RANDOM:
            if (debug) printf("BUSCA RANDÔMICA: Instância %d. Função objetivo: %s (%s), até %d iterações.\n\n",   instptr->inst, inst_brief[optobj], inst_descriptions[optobj], sort_number);
            melhor_solucao = (int *)malloc(sizeof(int)*instptr->nordens);

            for (i=0;i<sort_number;i++) {
           		if (debug) printf("Sorteio %4d. ", i+1);
    
       		    sort_inst(instptr,sort_method);
                calc_inst(instptr);
    
                if (debug) {
                    printf("Inst: %3d Obj: %5d. Seq: ", instptr->inst, inst_paramval(instptr,optobj));
                    DL_FOREACH(instptr->ohead,ordemptr)
                        printf("%2d ", ordemptr->ordem);
                }
            
                if (i == 0 || inst_paramval(instptr,optobj) < melhor_objetivo) {
                    melhor_objetivo = inst_paramval(instptr,optobj); 
                    if (debug) printf("(melhor objetivo)");
                            
                    j=0;
                    DL_FOREACH(instptr->ohead,ordemptr) {
                        melhor_solucao[j] = ordemptr->ordem;
                        j++;
                    }
                }
    
            if (debug) printf("\n");
    
            }
    
            if (debug) printf("\nMelhor solução objetivo: %d. Seq: ", melhor_objetivo);
    
            ordemptr = NULL;
            for (i=0;i<instptr->nordens;i++) {
            DL_SEARCH_SCALAR(instptr->ohead,ordemptr,ordem,melhor_solucao[i]);
                if (!ordemptr)
                    continue;
                ordemptr->seq = i+1;
                if (debug) printf("%d ", ordemptr->ordem);
            }
        
            if (debug) printf("\n\n");
    
            sort_inst(instptr,SORT_SEQ);
            calc_inst(instptr);
            
            break;
            
        case SORT_SEQ:
            if (debug) printf("SEQUÊNCIA IMPOSTA: Instância %d. Função objetivo: %s (%s).\n\n",   instptr->inst, inst_brief[optobj], inst_descriptions[optobj]);
            sort_inst(instptr,sort_method);
            calc_inst(instptr);
            break;
            
        case SORT_EDD:
            if (debug) printf("EDD: Instância %d. Função objetivo: %s (%s).\n\n",   instptr->inst, inst_brief[optobj], inst_descriptions[optobj]);
            sort_inst(instptr,sort_method);
            calc_inst(instptr);
            break;

        case SORT_TWKR_BY_TIS:
        case SORT_W_LBE_LBT:        
            if (debug && (optobj == SORT_TWKR_BY_TIS)) printf("TWKR-BY-TIS: Instância %d. Função objetivo: %s (%s).\n\n",   instptr->inst, inst_brief[optobj], inst_descriptions[optobj]);
            if (debug && (optobj == SORT_TWKR_BY_TIS)) printf("W(LBE+LBT): Instância %d. Função objetivo: %s (%s).\n\n",   instptr->inst, inst_brief[optobj], inst_descriptions[optobj]);
            
            sort_inst(instptr,sort_method);
            calc_inst(instptr);
            break;
            
        case SORT_ENUM:            
            if (debug) printf("BUSCA EXAUSTIVA: Instância %d. Função objetivo: %s (%s), até %d iterações.\n\n", instptr->inst, inst_brief[optobj], inst_descriptions[optobj], sort_number);
            melhor_solucao = (int *)malloc(sizeof(int)*instptr->nordens);

            sequencia = (int *)malloc(sizeof(int)*instptr->nordens);
            
            for (i=1; i <= instptr->nordens; i++)
                sequencia[i-1] = i;

            ovec = (ordens**)malloc(instptr->nordens*sizeof(ordens*));
            i = 0;
            DL_FOREACH(instptr->ohead, ordemptr)
                ovec[i++] = ordemptr;

            i=0;
            do {
           		if (debug) printf("Sequência %4d. ", i+1);

                for (j=0; j < instptr->nordens; j++)
                    (ovec[j])->seq = sequencia[j];
  
       		    sort_inst(instptr,SORT_SEQ);
                calc_inst(instptr);
    
                if (debug) {
                    printf("Inst: %3d Obj: %5d. Seq: ", instptr->inst, inst_paramval(instptr,optobj));
                    DL_FOREACH(instptr->ohead,ordemptr)
                        printf("%2d ", ordemptr->ordem);
                }
            
                if (i == 0 || inst_paramval(instptr,optobj) < melhor_objetivo) {
                    melhor_objetivo = inst_paramval(instptr,optobj); 
                    if (debug) printf("(melhor objetivo)");
                            
                    j=0;
                    DL_FOREACH(instptr->ohead,ordemptr) {
                        melhor_solucao[j] = ordemptr->ordem;
                        j++;
                    }
                }

            if (debug) printf("\n");

            i++;
            } while (next_perm(sequencia, instptr->nordens) && (i <= sort_number || sort_number == 0));
            
            free(ovec);
    
            if (debug) printf("\nMelhor solução objetivo: %d. Seq: ", melhor_objetivo);
    
            ordemptr = NULL;
            for (i=0;i<instptr->nordens;i++) {
            DL_SEARCH_SCALAR(instptr->ohead,ordemptr,ordem,melhor_solucao[i]);
                if (!ordemptr)
                    continue;
                ordemptr->seq = i+1;
                if (debug) printf("%d ", ordemptr->ordem);
            }
        
            if (debug) printf("\n\n");
    
            sort_inst(instptr,SORT_SEQ);
            calc_inst(instptr);
            break;
            
        case SORT_ANNL:
            if (debug) printf("SIMULATED ANNEALING: Instância %d. Função objetivo: %s (%s), até %d iterações.\n\n", instptr->inst, inst_brief[optobj], inst_descriptions[optobj], sort_number);

            solve(instptr, optobj);
            
            
        default:
            break;
    }
}

void print_help(int rcode) {
	int i;

	printf("Uso: %s [-h] [-v] [-f ARQUIVO] [-o ARQUIVO] [-i SEQ] -s [METODO]\n\n", PACKAGE);

	printf("  -h\t\timprime esta ajuda e sai\n\n");

	printf("  -v\t\tconfigura saída detalhada\n");
	printf("  -f\t\tconfigura entrada (padrão: stdin)\n");
    printf("  -p\t\tplotar gráfico de gantt\n");

	printf("  -i SEQ\tindica número da sequência (padrão: todas)\n");
	printf("  -s METODO\tmetodo de ordenação (padrão: seq)\n\n");
    printf("  -o OBJ\tfunção objetivo (padrão: atraso ponderado)\n\n");
	
    printf("  -c NUM\tnúmero de sorteios (método rand. padrão: sorteio simples)\n\n");
    printf("  -r\t\tReordenar seqüência se inviável (padrão: atrasar ordens)\n\n");

	printf("Métodos de sequenciamento disponíveis:\n");
	for (i=0; i<sort_num_methods; i++)
		printf("  %s\t\t%s\n", sort_methods[i], sort_descriptions[i]);

	printf("\nFunções objetivo disponíveis:\n");
	for (i=2; i<inst_num_params; i++)
		printf("  %d\t\t%s (%s)\n", i-1, inst_descriptions[i], inst_brief[i]);

	exit(rcode);
}

