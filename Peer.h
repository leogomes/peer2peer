#ifndef PEER_H
#define PEER_H

#define TAM_MAX_SEG 1000

int abreConexao(IdPeer& peer);

void readCmdLine();

void * SelecionaDescritores(void *args);

IdPeer sockToIdPeer(int socket);

#endif
