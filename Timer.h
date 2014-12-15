#include <stdio.h>
#include <stdlib.h> 
#include <vector>
#include <math.h>
#include <queue> 
#include <sys/time.h>

/** Definição de estruturas TIMER **/
#ifndef TIMER_H
#define TIMER_H

// Estrutura que será enfileirada.
typedef struct TIPO_TIMER {
	int tipo;
	short indiceVizinho;
	long segundo;
	long milisegundo;
}TTimer;

struct comp_tempo : public std::binary_function<TTimer, TTimer, bool>
{
	bool operator()(const TTimer x, const TTimer y) {
		if( x.segundo == y.segundo )
			return x.milisegundo > y.milisegundo;
		else
			return x.segundo > y.segundo;
	}
};

struct timeval * RetornaTimer();
short RetornaIndiceVizinho();
int RetornaTipoTimer();
void SetTimer(long tempoParam, int tipoParam, short indiceVizinho);
int TimerTratado();
void imprimeTimer();

#endif
