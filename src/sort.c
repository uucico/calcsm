#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "sort.h"
#include "calcsm.h"

/* definicoes dos tipos de sequenciamento */
char *sort_methods[] = { "rand", "seq", "disp", "annl", "edd", "enum", "twkr" };
char *sort_descriptions[] = { 	"randômico",
				"sequência imposta (coluna 9 da entrada)",
				"regra de liberação W(LBE+LBT)",
				"simmulated annealing",
				"earliest due date first",
				"enumeration - exhaustive search",
				"regra de liberação TWKR-BY-TIS" };

int sort_num_methods = 7;
int sort_number;

/* obtem metodo de sort a partir da linha de comando */
int switch_sort(char *sptr) {
	int i;

	for (i=0; i<sort_num_methods; i++)
		if (!strcmp(sptr, sort_methods[i]))
    		return i;
 
	return SORT_NOMETHOD;
}

/* compara starrt da ordem de a e b */
int ordemstart(ordens *a, ordens *b) {
    if (a->startt > b->startt)
        return 1;

    if (a->startt == b->startt)
        return (a->d < b->d);       /* desempate: menor due date */

    return 0;
}

/* compara numero da ordem de a e b */
int ordemcmp(ordens *a, ordens *b) {
	return a->ordem > b->ordem;
}

/* compara numero da sequencia de a e b */
int ordemseq(ordens *a, ordens *b) {
	return a->seq > b->seq;
}

/* compara se due date de ordem a eh maior que de b */
//TODO: CRITERIO DE DESEMPATE
int ordemedd(ordens *a, ordens *b) {
	if (a->d > b->d)
		return 1;
	else
	return a->d > b->d;
}

void sort_inst(instancias *insthead, int type) {
	int i, n, r, temp, T, melhor_indice, TIS;
	float D;
	ordens *optr, *optrtemp;
	ordens **ovec;

    optr = insthead->ohead; /* aponta para primeira ordem */
    n = insthead->nordens;      /* obtem numero de ordens */
    
	switch(type) {
	    case SORT_W_LBE_LBT:
          
            for (i=1; i <= n; i++) {
                T = insthead->Cmax;
                melhor_indice = -10000;
                
                optrtemp = NULL;
                
                do {
                
                    DL_FOREACH(insthead->ohead, optr) {
                        if (optr->realloc == 1 || optr->r > T)
                            continue;
                        
                       
                        temp = T + optr->p - optr->d;
                        
                        D = (temp > 0) ? temp*optr->wT : 0;
                        
                        temp = optr->d - T - optr->p;
                        
                        D -= (temp > 0) ? temp*optr->wE : 0;

                        if (debug) printf("W(LBE+LBT): Sequência %d Ordem %d LBSC = %d, d = %d, wE = %d, wT = %d, d_j = %2.2f.\n", i, optr->ordem, T + optr->p, optr->d, optr->wE, optr->wT, D);
                    
                        if (D > melhor_indice) {
                            optrtemp = optr;
                            melhor_indice = D;
                        }
                    }
                    T++;
                } while (optrtemp == NULL);
                
                if (debug) printf("W(LBE+LBT): Alocando ordem %d na sequência %d.\n", optrtemp->ordem, i);
                optrtemp->realloc = 1;
                optrtemp->seq = i;
                T = T + optrtemp->p;
            }
            
            DL_FOREACH(insthead->ohead, optr)
                optr->realloc = 0;
            
  			DL_SORT(insthead->ohead, ordemseq);
            
            if (debug) printf("Fim.\n\n");
            break;	
	
	
	    case SORT_TWKR_BY_TIS:
          
            for (i=1; i <= n; i++) {
                T = insthead->Cmax;
                melhor_indice = -1;
                
                optrtemp = NULL;
                
                do {
                
                    DL_FOREACH(insthead->ohead, optr) {
                        if (optr->realloc == 1 || optr->r > T)
                            continue;
                        
                        TIS = T - optr->r; /* time in system para job 1 passo */

                        if (TIS == 0) TIS = 1;
                                        
                        D = (optr->p / TIS ) / (optr->w + (optr->wT / optr->wE ));
                        if (debug) printf("TWKR-BY-TIS: Sequência %d Ordem %d TIS = %d, d_j = %2.2f.\n", i, optr->ordem, TIS, D);
                    
                        if (melhor_indice < 0 || D < melhor_indice) {
                            optrtemp = optr;
                            melhor_indice = D;
                        }
                    }
                    T++;
                } while (optrtemp == NULL);
                
                if (debug) printf("TWKR-BY-TIS: Alocando ordem %d na sequência %d.\n", optrtemp->ordem, i);
                optrtemp->realloc = 1;
                optrtemp->seq = i;
                T = T + optrtemp->p;
            }
            
            DL_FOREACH(insthead->ohead, optr)
                optr->realloc = 0;
            
  			DL_SORT(insthead->ohead, ordemseq);
            
            if (debug) printf("Fim.\n\n");
            break;
	    
		case SORT_RANDOM: 
			/* random sort */
			/**
			 * Cria vetor onde cada item aponta para cada ordem
			 * isto eh feito para evitar atravessar a lista a cada sort.
			 * Primeiro atravessa a lista uma vez setando os ponteiros.
			 */
			ovec = (ordens**)malloc(n*sizeof(ordens*));
            i = 0;
            DL_FOREACH(insthead->ohead, optr)
                ovec[i++] = optr;

			/* loop numero de ordens para sortear */
			for (i = 0; i < n; i++) {
			    r = rnd(insthead->nordens); /* obtem numero randomico */
			    
				/* troca numero de sequencia da ordem[i] com orden[r] */
//				printf("%d<->%d ", ovec[i]->seq, ovec[r]->seq);
				temp = ovec[i]->seq;
				ovec[i]->seq = ovec[r]->seq;
				ovec[r]->seq = temp;
			}

			free(ovec); /* elimina vetor */
			
            /* FALLTHROUGH PARA BAIXO PARA SORTEAR COM SEQUENCIA IMPOSTA */
		case SORT_SEQ:
			/* sequencia imposta */
			DL_SORT(insthead->ohead, ordemseq);
			/* atualiza numero de sequencia */
			break;
		case SORT_ORDER:
			/* sequencia da ordem */
			DL_SORT(insthead->ohead, ordemcmp);
			/* atualiza numero de sequencia */
			break;			
    	case SORT_STARTT:
    	    /* para ser usado apenas quando ja sorteado */
  			DL_SORT(insthead->ohead, ordemstart);
    	    break;
		case SORT_EDD:
			/* sort by earliest due date */
//			printf("head antes: %p, o=%i",insthead->ohead, insthead->ohead->ordem);
			DL_SORT(insthead->ohead, ordemedd);
			i=1;
			DL_FOREACH(insthead->ohead,optr) {
				optr->seq=i++;
			}
//			printf(" head depois: %p, o=%i.\n",insthead->ohead, insthead->ohead->ordem);	
			break;


		default:
			break;
	}
}
