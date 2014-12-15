#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <sys/select.h>
#include <sys/time.h>
#include "mysock.h"
#include "Estruturas.h"
#include "Peer.h"
#include "Eventos.h"
#include "Timer.h"

//#define DEBUG
//#define DEBUG1
#define DEBUG_JOINR
#define DEBUG_DISTANCIA
//#define DEBUG_ROT
#define DEBUG_VIZINHOS
//#define DEBUG_HELLO
#define DEBUG_SEARCH


#ifndef PRINCIPAL_H
#define PRINCIPAL_H


#define TAM_STR_IP 15


// Argumentos das funcoes passadas para as threads.
typedef struct {
	int  listenfd;					// Descritor de socket que escuta requisicoes
	int  portaConnect;				// Porta para conectar, caso o noh naum seja principal
	char ipConnect[TAM_STR_IP + 1];	// Ip do noh para conectar, caso naum seja principal
} TpArgumento;


// Guarda todas as informa�es pertinentes a aplica�o.

typedef struct {
	bool		principal;
	int			intervaloHello;
	int			intervaloHelloR;
	int			intervaloTimeout;
	int			tempoLive;
	IdPeer		meuIp;
	IdPeer		noPrincipalIp;
	char 		meuIpString[TAM_STR_IP + 1];
	int			sockPrincipalConectado;

} TDadosAplicacao;

#endif

