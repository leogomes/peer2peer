/* UFRJ - DCC - Teleprocessamento e Redes - 2004/2
	Fabio Pereira Sarmento - DRE:
	Gustavo Zopelari Nasu - DRE: 098212015
	Leonardo Freitas Gomes - DRE: 098211336
	Ricardo Chiganer Lilenbaum - DRE: 101133019 */
	
#ifndef _MYSOCK_H_
#define _MYSOCK_H_

#include<stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <errno.h>
#include <utmp.h>
#include "Estruturas.h"


/* ---------- constantes ---------- */

#define MAXBACKLOG 10							//tamanho da maximo da fila de backlog
#define MAX_NOME_ARQ 256						//tamanho maximo de nome de arquivo


/* ----- definicao do tipo porta ----- */

typedef unsigned short u_port_t;


/* ---------- prototipos de funcoes para comunicacao via socket ---------- */

int u_open(u_port_t porta, char *ipEscuta);
int u_close(int sockfd);
int u_accept(int sockfd, char *hostn);
int u_connect(u_port_t porta, char *inetp);
ssize_t u_recv(int fd, void *buf, size_t nbyte);
ssize_t u_send(int fd, void *buf, size_t nbyte);
int lePacote(int fd, Header& header, char * buffP);
bool escrevePacote(int fd, Header& header, char * buffP);


#endif
