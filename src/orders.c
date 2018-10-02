#include <stdio.h>

#include "orders.h"

instancias *loadorders(FILE *fp) {
   	char *ptr, buff[8192];  /* FIXME: buffer estatico... */
   	ordens otmp, *ordemptr;
   	instancias *instptr, *insthead=NULL;
   	
	while (fgets(buff,sizeof(buff),fp)) {
		ptr = &buff[0];
		if (*ptr == '#')    /* ignora linha iniciada com % */
		    continue;
		
		/* zera temporario e le dados */
		memset(&otmp,0,sizeof(ordens));
		if (sscanf(ptr, "%d %d %d %d %d %d %d %d %d", &otmp.inst, &otmp.ordem,
    		&otmp.r, &otmp.d, &otmp.w, &otmp.wT, &otmp.wE, &otmp.p, &otmp.seq)
    		!= 9)
			continue;       /* ignora linha invalida */

		ordemptr = (ordens*)malloc(sizeof(ordens)); 		/* aloca ordem */
		if (!ordemptr) {
			perror("erro ao alocar ordem");
			return NULL;
		}
		
		memcpy(ordemptr, &otmp, sizeof(ordens));            /* copia dados */

		/* verifica existencia da instancia */
		DL_SEARCH_SCALAR(insthead,instptr,inst,ordemptr->inst);
		if (!instptr) {
			/* cria nova instancia */
			instptr = (instancias*)malloc(sizeof(instancias));
			if (!instptr) {
				perror("erro ao alocar instancia");
				return NULL;
			}
				
			memset(instptr,0,sizeof(instancias));
			DL_APPEND(insthead, instptr);

			instptr->inst=ordemptr->inst;
		}

	    DL_APPEND(instptr->ohead, ordemptr);
		ordemptr->instptr = instptr;
		instptr->nordens++;
	}
	
    return (instancias *)insthead;
}

void dump_ordens_gnuplot(FILE *fd, instancias *insthead, int inst) {
	instancias *ptr;
	ordens *optr;
    
    DL_FOREACH(insthead,ptr) {
		if (inst == INSTANCE_ANY || ptr->inst == inst)
			DL_FOREACH(ptr->ohead,optr)
				if (optr->inst == ptr->inst) {
                
					fprintf(fd,"I%d %d %d J%d\n", optr->inst, optr->startt,
					    optr->startt + optr->p, optr->ordem);
		        }
	}
}

void dump_ordens(instancias *insthead, int inst) {
	instancias *ptr;
	ordens *optr;
    char c;
/*
                release    due     proc             tard   earl
  inst     ord  time       date    time    weight   wgt    wgt
 123456X123456XX123456789XX1234567X1234567X1234567XX123456X123456
     1     1  0         20              1         1         1         5
     2     1  0         8               1         1         1         4
*/

    printf("                 R E S U M O   D A   S E Q U Ê N C I A\n\n");
    
    printf("                    start   release    due     proc    job weights\n");
    printf("   inst   ord  seq  time    time       date    time    w_j w_T w_E\n");

    DL_FOREACH(insthead,ptr) {
		if (inst == INSTANCE_ANY || ptr->inst == inst)
			DL_FOREACH(ptr->ohead,optr)
				if (optr->inst == ptr->inst) {
                    c = ' ';
                    if (optr->startt+optr->p > optr->d)
                        c = 'T';
                    if (optr->startt+optr->p < optr->d)
                        c = 'E';
                    if (optr->realloc == 1)
                        c = 'R';
                
					printf(" %6d  %4d %4d%c %-6d  %-9d  %-7d %-7d %3d %3d %3d\n",
					    optr->inst,
						optr->ordem,
						optr->seq,
						c,
    					optr->startt,
						optr->r,
                        optr->d,
                        optr->p,
                        optr->w,
                        optr->wT,
                        optr->w);
		        }
	}
}

/*
 * Atualiza dados da instancia para a ordem na posicao conforme segue
 */
void commit_order(instancias *insthead, ordens *optr) {
    int C, T=0, E=0, L=0;

    optr->startt = insthead->Cmax;  /** startt da ordem é makespan até o momento 
                                      */
    C = insthead->Cmax + optr->p;   /** calcula completion time */
    L = C - optr->d;                /** calcula lateness */  

//        printf("Ordem %d Due date = %d Completion Time = %d. L=%d\n", optr->ordem, optr->d, C, L);
        
            /** verifica atraso ou adiantamento */
    if (L>0) {          
        /* ordem atrasada */
        T=L;
        //printf("ordem %d atrasada em %d! peso w=%d\n", optr->ordem, T, optr->wT);
        insthead->sum_U += 1;
        insthead->sum_wU += optr->wT;
        insthead->sum_T += L;
        insthead->sum_wT += optr->wT * L;
        
        /** Verifica se é lateness maior que o Lmax até o momento. */
        if (L > insthead->Lmax)
            insthead->Lmax = L;
        if (L > insthead->Tmax)
            insthead->Tmax = L;
            
    } else {
        /* Adiantada ou em dia. Calcula earliness. */

        E = optr->d - C;
        insthead->sum_E += E;
        insthead->sum_wE += optr->wE * E;

        if (E > insthead->Emax)
            insthead->Emax = E;
//        printf("ordem %d adiantada em %d! peso w=%d, max ant=%d\n", optr->ordem, E, optr->wE, insthead->Emax);
    }
    
    /* modifica instancia */
    insthead->Cmax = C;
    insthead->sum_C += C;
    insthead->sum_wC += optr->w * C;
    insthead->sum_wCmR += optr->w * (C - optr->r);
    insthead->sum_wTpwE += (optr->wT * T) + (optr->wE * E);
//    printf("wt=%d, t=%d, we=%d, e=%d, res=%d\n", optr->wT, T, optr->wE, E, insthead->sum_wTpwE);

}

/** Verifica se podemos alocar a ordem na instância na posição indicada.
  */
inline int check_constraints(instancias *insthead, ordens *optr) {

    /** Makespan até agora é quando se deve iniciar a proxima ordem, desde que
      * seja plausivel.
      */     
    if (insthead->Cmax < optr->r)
        return FALSE;           /** Falso. Ordem não disponível no momento */
    return TRUE;                /** Verdadeiro. Ordem disponível */
}

/* insere a depois de b */
void insert_after(ordens *a, ordens *b) {

    if (b) {
        a->next = b->next;
        a->prev = b->prev;
        
        b->next = a;
        if (a->next)
            a->next->prev = a;
   }
 
}
/* troca posicao das ordens a e b */
void swap_orders(ordens *a, ordens *b) {
    ordens *prevtemp, *nexttemp;
    
    prevtemp = a->prev;
    nexttemp = a->next;

    a->prev = b->prev;    
    a->next = b->next;

    b->prev = prevtemp;
    b->next = nexttemp;

    if (b->prev)
        b->prev->next = b;
    if (b->next)
        b->next->prev = b;

    if (a->prev)
        a->prev->next = a;
    if (a->next)
        a->next->prev = a;
}

