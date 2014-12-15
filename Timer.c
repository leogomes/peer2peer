#include "Timer.h"


// Estrutura do timer
struct timeval timerGlobal;

// Guarda o tipo do timer atual que esta sendo tratado.
// É inicializado com -1, assim sabemos que não existe nenhum
// timer 'ativo'. 
int TipoTimerGlobal = -1;
int IndiceVizinho = -1;

// Função predicado. Esta função será  utilizada na 'priority_queue' para
// manter sempre o próximo evento de timer no topo da fila.
// Fila de eventos dos 'timers'.
std::priority_queue<TTimer, std::vector<TTimer>, comp_tempo> FilaTimer;


void EncontraTempoAtual(long * tempoSegundo, long * tempoMilisegundo);


/******************************************************************
			FUNÇÕES UTILITARIAS
 ******************************************************************/

// Função: EncontraTempoAtual
// @param: long * - retorna os segundos atuais.
// @param: long * - retorna os milisegundos atuais.
// @retor: void - Nada.
// Descrição: Encontra o tempo atual

void EncontraTempoAtual(long * tempoSegundo, long * tempoMilisegundo)
{
	*tempoSegundo = time(NULL);
	*tempoMilisegundo = 0; 
}
 
void imprimeTimer()
{
	if(TipoTimerGlobal != -1) {
		fprintf(stdout, "Timer: %ld s %ld ms\n",
			timerGlobal.tv_sec,
			timerGlobal.tv_usec);
	}
}

/******************************************************************
			FUNÇÕES DE TIMER
 ******************************************************************/

 // Função: RetornaTimer
// @retor: struct timeval * - Endereco com a estrutura do timer ou null se nao existir timer na fila..
// Descrição: Retorna o proximo timer.

struct timeval * RetornaTimer() {
	if(TipoTimerGlobal == -1)
		return NULL;
		
	return &timerGlobal;
}


// Função: RetornaIndiceVizinho
// @retor: short - Retorna indice do vizinho.
// Descrição: Retorna o indice do vizinho do timer disparado.

short RetornaIndiceVizinho()
{
	if(TipoTimerGlobal == -1)
		return -1;
		
	return IndiceVizinho;
}


// Função: RetornaTipoTimer
// @retor: short - Retorna o tipo do timer.
// Descrição: Retorna o tipo de timer corrente.

int RetornaTipoTimer()
{
	return TipoTimerGlobal;
}


// Função: SetTimer
// @param: long - tempo (em segundos) que deve ser disparado o evento de timer.
// @param: int - tipo do timer (do evento de timer).
// @retor: void - Nada.
// Descrição: Gerência a sequência de entrada dos eventos de timer na fila.

void SetTimer(long tempoParam, int tipoParam, short indiceVizinho)
{
	long segAtual = 0;
	long milisegAtual = 0;
	TTimer timer;
	
	if(tempoParam < 0)
		tempoParam = 0;
	
	if(TipoTimerGlobal == -1) {
		timerGlobal.tv_sec = tempoParam;
		timerGlobal.tv_usec = 0;
		TipoTimerGlobal = tipoParam;
		IndiceVizinho = indiceVizinho;
	}
	else {
		EncontraTempoAtual(&segAtual, &milisegAtual);
	
		if(tempoParam <= timerGlobal.tv_sec) {
			timer.segundo = timerGlobal.tv_sec + segAtual;
			timer.milisegundo = timerGlobal.tv_usec + milisegAtual;
			timer.tipo = TipoTimerGlobal;
			timer.indiceVizinho = IndiceVizinho;
			
			timerGlobal.tv_sec = tempoParam;
			timerGlobal.tv_usec = 0;
			IndiceVizinho = indiceVizinho;
			TipoTimerGlobal = tipoParam;
		}
		else {
			timer.segundo = tempoParam + segAtual;
			timer.milisegundo = milisegAtual;
			timer.tipo = tipoParam;
			timer.indiceVizinho = indiceVizinho;
		}
		
		FilaTimer.push(timer);
	}
}


// Função: TimerTratado
// @retor: int - retorna zero se não há mais nenhum evento de timer na fila. No
//               nosso sistema (p2p) isto deve ser tratado.
// Descrição: Gerência o fluxo de eventos de timer na fila.

int TimerTratado()
{
	long segAtual;
	long milisegAtual;
	TTimer timer;
	
	if(timerGlobal.tv_sec == 0) {
		if(FilaTimer.size())
		{
			EncontraTempoAtual(&segAtual, &milisegAtual);

			timer = FilaTimer.top();
			FilaTimer.pop(); 
			
			timerGlobal.tv_sec = timer.segundo - segAtual;
			timerGlobal.tv_usec = timer.milisegundo - milisegAtual;
			TipoTimerGlobal = timer.tipo;
			IndiceVizinho = timer.indiceVizinho; 
			
			if(timerGlobal.tv_sec < 0)
				timerGlobal.tv_sec = 0;
			if(timerGlobal.tv_usec < 0)
				timerGlobal.tv_usec = 0;
			
			return 1;
		}
		else
		{
			TipoTimerGlobal = -1;
		
			return 0;
		}
	}
	
	return 1;
}

/*

// ESTE MAIN FOI INSERIDO AQUI SOMENTE PARA TESTAR O CODIGO
// RESULTADO ESPERADO (S/ ERRO):
// h - tempo: 2
// j - tempo: 3
// s - tempo: 4
// h - tempo: 5
// h - tempo: 7
// FIM - tempo: 9

int main()
{
	long segundo, segFixo;
	long milisegundo, milisegFixo;
	int nready;
	fd_set rset;
	SetTimer(5, TIMER_HELLO);
	SetTimer(3, TIMER_JOINRESP);
	SetTimer(7, TIMER_HELLO);
	SetTimer(2, TIMER_HELLO);
	
	EncontraTempoAtual(&segFixo, &milisegFixo);

	for(;;) {
		while((nready = select(1, &rset, NULL, NULL, &timerGlobal)) == -1);
		
		EncontraTempoAtual(&segundo, &milisegundo);

		switch(TipoTimerGlobal)
		{
			case TIMER_HELLO:
				printf("h - tempo: %d\n", (int)segundo-segFixo);
				break;
			case TIMER_JOINRESP:
				printf("j - tempo: %d\n", (int)segundo-segFixo);
				SetTimer(1,TIMER_SEARCHRESP);
				break;
			case TIMER_SEARCHRESP:
				printf("s - tempo: %d\n", (int)segundo-segFixo);
				SetTimer(5, 10);
				break;
			case 10:
				printf("FIM - tempo: %d\n", (int)segundo-segFixo);
				exit(0);
		}
		TimerTratado();
	}
}*/
