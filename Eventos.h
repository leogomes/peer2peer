#ifndef EVENTOS_H
#define EVENTOS_H


void onEntrada();

void onSaida();

void onGetArquivo(char *);

void onImprimeTabelaVizinho();

void onRoteamento();

void trataJoin(int , Header &, char *);
void trataJoinResponse(int , Header &, char *);
void trataHello(int , Header &, char *);
void trataSearch(int , Header& , char * );
void trataSearchResponse(int , Header& , char * );
void trataGet(int , Header& , char * );
void trataGetResponse(int , Header& , char * );

void *enviaArquivo(void *);

void disparaHello();

void trataTimerHello();
void trataTimerJoinR();
void trataTimerSearchR();
void trataTimerGetR();

#endif
