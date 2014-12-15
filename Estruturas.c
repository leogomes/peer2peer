#include "Principal.h"
#include "Estruturas.h"
#include <list>


RegVizinho vizinhos[MAX_VIZINHOS];
TempRegVizinho tempVizinhos[MIN_VIZINHOS];
std::list<RegRoteamento> TabelaRoteamento;
std::list<RegSocket> TabelaSocket;

RegSearch tempSearch;
RegGetResponse getResponse;

NumeroSequencia MsgsValidas[QTD_TIPOS_MSGS_RESP];
extern TDadosAplicacao DadosAplicacao;

// Fun�o: CreateHeader
// @param: Header * - retorna o cabe�lho.
// @param: char - informa o tipo de mensagem.
// @param: IdPeer - informa o destino.
// @retor: void - Nada.
// Descri�o: Cria o 'cabe�lho' de uma mensagem.

void createHeader(Header *h, char tipoMsg, IdPeer destino)
{
	h->tipoMsg = tipoMsg;
	h->destino = destino;
	h->remetente = DadosAplicacao.meuIp;
	h->numSeq = numSequencia(tipoMsg);
}


// Fun�o: IdPeer2StrIp
// @param: char * - retorna o ip do IdPeer.
// @param: IdPeer - estrutura que cont� o ip.
// @retor: void - Nada.
// Descri�o: Converte o ip do formato 'IdPeer' para string.

void IdPeer2StrIp(char * str, IdPeer id)
{
	sprintf(str, "%d.%d.%d.%d", id.oc1, id.oc2, id.oc3, id.oc4);
}


// Fun�o: String2IdPeer
// @param: char * - ip (string).
// @retor: IdPeer - retorna o ip no formato 'IdPeer'.
// Descri�o: Converte o ip do formato string para 'IdPeer'.

IdPeer string2IdPeer(char *id)
{
	IdPeer id_convert;

	id_convert.oc1= atoi(strtok(id, "."));
	id_convert.oc2= atoi(strtok(NULL, "."));
	id_convert.oc3= atoi(strtok(NULL, "."));
	id_convert.oc4= atoi(strtok(NULL, ":"));
	id_convert.porta= atoi(strtok(NULL, "\n"));

	return id_convert;
}


// Fun�o: CalcDistancia
// @param: IdPeer * - endere� ip 1.
// @retor: IdPeer * - endere� ip 2.
// Descri�o: Calcula a dist�cia entre dois ip.

unsigned char calcDistancia(IdPeer *peer1, IdPeer *peer2)
{
	tipo_porta porta;
	unsigned soma_idx, soma_idy;
	register unsigned short i;

	/* para cada no calcula a soma dos bytes da identificacao do no mod 20 */
	porta.porta_int = peer1->porta;
	soma_idx = peer1->oc1 + peer1->oc2 + peer1->oc3 + peer1->oc4;
	for(i=0; i<sizeof(unsigned); i++) soma_idx += porta.porta_byte[i];
	soma_idx = soma_idx % 20;

	porta.porta_int = peer2->porta;
	soma_idy = peer2->oc1 + peer2->oc2 + peer2->oc3 + peer2->oc4;
	for(i=0; i<sizeof(unsigned); i++) soma_idy += porta.porta_byte[i];
	soma_idy = soma_idy % 20;

	/* distancia = diferenca entre os valores */
	if(soma_idx > soma_idy){ 
		#ifdef DEBUG_DISTANCIA
			printf("Retorno de calcDistancia: %d\n", (soma_idx - soma_idy));
		#endif
		return (unsigned char)soma_idx - soma_idy;
	}
	else{
		#ifdef DEBUG_DISTANCIA
			printf("Retorno de calcDistancia: %d\n", (soma_idy - soma_idx));
		#endif
		 return (unsigned char)soma_idy - soma_idx;
	}
}


// Fun�o: NumSequencia
// @param: char - tipo de mensagem.
// @retor: short - nmero de sequ�cia.
// Descri�o: Retorna o nmero de sequ�cia para o tipo de mensagem correspondente, ou -1 se 
//			  tipo de for inv�ido.

short numSequencia(unsigned char tipoMsg)
{
	static short join=0;
	static short hello=0;
	static short search=0;
	static short get=0;
	
	switch(tipoMsg)
	{
		case JOIN:
			join= ++join % 5000;
			return join;
		case HELLO:
			hello= ++hello % 5000;
			return hello;
		case SEARCH:
			search= ++search % 5000;
			return search;
		case GET:
			get= ++get % 5000;
			return get;
		default: //nenhum dos tipos definidos => erro
			//fprintf(stderr, "Erro em 'numSequencia'.\n");
			return -1;
	}
}


bool ehMensagemRoteada(char tipoMsg)
{
	switch(tipoMsg)
	{
		case JOIN:
		case SEARCH:
		case GET:
			return true;
		default: //nenhum dos tipos definidos => erro
			return false;
	}
}

// Funcao que recebe um socket e retorna o IdPeer de quem estah do outro lado
// TODO Faltar tratar erro !
IdPeer sockToIdPeer(int socket){
	
	int retval, porta;
	char ip[22];
	struct sockaddr_in name;
	socklen_t len = (socklen_t)sizeof(struct sockaddr);
	
	while( ( ( retval= getpeername(socket,(struct sockaddr *)&name, &len) ) == -1 ) && (errno == EINTR));
	
	strcpy(ip,  inet_ntoa(name.sin_addr));
	
 	printf("1a. tentativa ----->>>>> Ip encontrado pelo sockToIdPeer: %s  PORTA: %d --- MEU IP: %s\n",ip, name.sin_port, DadosAplicacao.meuIpString);

	if ( strcmp(DadosAplicacao.meuIpString,ip) ){
		
			while( ( ( retval= getsockname(socket,(struct sockaddr *)&name, &len) ) == -1 ) && (errno == EINTR));
			strcpy(ip,  inet_ntoa(name.sin_addr));
			printf("2a. tentativa ----->>>>> Ip encontrado pelo sockToIdPeer: %s --- PORTA: %d \n",inet_ntoa(name.sin_addr), name.sin_port);
	}

	porta = name.sin_port;
	
	sprintf(ip, "%s:%d", ip, porta );
	
	return string2IdPeer(ip);
}


/*************************
	Fun�es das tabelas
 *************************/

void InicializaTabelas(){
	
	InicializaTabelaTempVizinhos();
	InicializaTabelaVizinhos();

}

void InicializaTabelaTempVizinhos()
{
	for(int i=0; i<MIN_VIZINHOS ; i++)
	{
		memset( &tempVizinhos[i].vizinho, 0, sizeof(IdPeer));
		tempVizinhos[i].distancia = -1;
		tempVizinhos[i].sockfd = -1;
	}
}

void InicializaTabelaVizinhos()
{
	for(int i = 0; i < MAX_VIZINHOS; i++) {
		vizinhos[i].sockfd = -1;
		memset( &vizinhos[i].vizinho, 0, sizeof(IdPeer));
	}

}

void escolheVizinhos(const IdPeer& vizinho, int distancia, int sockfd)
{
	int i;
	
	// Verifica se o vizinho ja existe na 'Tabela de vizinhos'.
	if(buscaSocketVizinho(vizinho) != -1)	return;

	for(i = 0; i < MIN_VIZINHOS; i++){
		// Se jah estiver na tabela tempVizinhos
		if(IdPeerIguais(tempVizinhos[i].vizinho, vizinho)){
			return;
		}
	}
	
	// Verifica se existe uma posicao vazia.
	for(i = 0; i < MIN_VIZINHOS; i++){
		// Se a posicao da tabela de vizinhos estah livre OU a distancia recebida eh menor do que a da tabela
		if((tempVizinhos[i].distancia == -1) && !IdPeerIguais(tempVizinhos[i].vizinho, vizinho)){
			tempVizinhos[i].vizinho=vizinho;
			tempVizinhos[i].distancia = distancia;
			tempVizinhos[i].sockfd = sockfd;
			
			return;
		}
	}
	
	// Verifica se existe uma posicao com a distancia maior.
	for(i = 0; i < MIN_VIZINHOS; i++) {
		if(tempVizinhos[i].distancia  >  distancia) {
			tempVizinhos[i].vizinho=vizinho;
			tempVizinhos[i].distancia = distancia;
			tempVizinhos[i].sockfd = sockfd;
			
			return;
		}
	}
}


void defineVizinhos()
{
	int i;
	int j;

	// Procura para ver se os vizinhos não existem.
	for(i = 0; i < MAX_VIZINHOS; i++) {
		if(vizinhos[i].sockfd != -1) {
			for(j = 0; j < MIN_VIZINHOS; j++) {
				if((IdPeerIguais(vizinhos[i].vizinho, tempVizinhos[j].vizinho)) && (tempVizinhos[j].sockfd != -1)) {
					// Vizinho já existe na Tabela Principal de Vizinhos.
					tempVizinhos[j].sockfd = -1;
				}
			}
		}
	}
	
	// Insere vizinho da tabela temporaria para a tabela principal.
	for(j = 0; j < MIN_VIZINHOS; j++) {
		if(tempVizinhos[j].sockfd != -1) {
			for(i = 0; i < MAX_VIZINHOS; i++) {
				if(vizinhos[i].sockfd == -1){
					vizinhos[i].vizinho = tempVizinhos[j].vizinho;
					vizinhos[i].sockfd = tempVizinhos[j].sockfd;
					
					break;
				}
			}
		}
	}
}

int contaVizinhos(){

	int qtd=0;
	
	for( int i=0; i< MAX_VIZINHOS; i++){
		if(vizinhos[i].sockfd != -1) qtd++;
	}

	return qtd;
}

bool insereVizinho(const IdPeer& vizinho, int sockfd){
	
	// Verifica se o vizinho ja existe na 'Tabela de vizinhos'.
	if(buscaSocketVizinho(vizinho) != -1)	return true;

	for(int i = 0; i < MAX_VIZINHOS; i++){
		if(vizinhos[i].sockfd == -1){
			vizinhos[i].vizinho= vizinho;
			vizinhos[i].sockfd = sockfd;
			return true;
		}
	}
	return false;
}

bool removeVizinho(const IdPeer& vizinho){

	for(int i = 0; i < MAX_VIZINHOS; i++){
		if(IdPeerIguais(vizinhos[i].vizinho,  vizinho)){
			vizinhos[i].sockfd = -1;
			memset( &vizinhos[i].vizinho, 0, sizeof(vizinhos));
			return true;
		}
	}
	return false;
}

int buscaSocketVizinho(const IdPeer& vizinho){

	for(int i = 0; i < MAX_VIZINHOS; i++){
		if(IdPeerIguais(vizinhos[i].vizinho, vizinho) && (vizinhos[i].sockfd != -1)){
			return vizinhos[i].sockfd;
		}
	}
	return -1;
}

int buscaPosicaoVizinho(const IdPeer& vizinho){

	for(int i = 0; i < MAX_VIZINHOS; i++){
		if(IdPeerIguais(vizinhos[i].vizinho, vizinho) && (vizinhos[i].sockfd != -1)){
			return i;
		}
	}
	return -1;
}

bool buscaIdPeerVizinho(int sockfd, IdPeer &vizinho){

	for(int i = 0; i < MAX_VIZINHOS; i++){
		if(vizinhos[i].sockfd == sockfd){
			vizinho = vizinhos[i].vizinho;
			return true;
		}
	}
	return false;
}


void imprimeTabelaVizinhos()
{
	char sVizinho[TAM_STR_IP + 1];
	
	fprintf(stdout, "Tabela de Vizinhos:\n");
	
	for(int i = 0; i < MAX_VIZINHOS; i++){
		if(vizinhos[i].sockfd != -1) {
			
			IdPeer2StrIp(sVizinho, vizinhos[i].vizinho);
	
			fprintf(stdout, "Vizinho: %s\tSocket: %d\n",
				sVizinho,
				vizinhos[i].sockfd);
		}
	}
}

void imprimeTabelaVizinhosTemp()
{
	char sVizinho[TAM_STR_IP + 1];
	
	fprintf(stdout, "Tabela de Vizinhos Temp:\n");
	
	for(int i = 0; i < MIN_VIZINHOS; i++){
		if(tempVizinhos[i].sockfd != -1) {
			
			IdPeer2StrIp(sVizinho, tempVizinhos[i].vizinho);
	
			fprintf(stdout, "Vizinho: %s\tSocket: %d\n",
				sVizinho,
				tempVizinhos[i].sockfd);
		}
	}
}


/*******
 * FUNCOES DA TABELA DE SEARCH 
 * *****/
 
void setNomeArquivoSearch(char *nomeArq)
{
	strcpy(tempSearch.nomeArquivo, nomeArq);
}

void inicializaSearch()
{
	memset(&tempSearch.peer, 0, sizeof(IdPeer));
	tempSearch.distancia = 65000;
	strcpy(tempSearch.nomeArquivo, "");
}

void escolheSearch(IdPeer &peer, int distancia)
{
	char tmp[TAM_STR_IP + 1];

	IdPeer2StrIp(tmp, peer);

	printf("\n********** ESCOLHENDO SEARCH *************\n");
	printf("Remetente: %s\n", tmp);
	printf("distancia: %d\n", distancia);
	
	IdPeer2StrIp(tmp, tempSearch.peer);
	printf("Remetente: %s\n", tmp);
	printf("distancia: %d\n", tempSearch.distancia);
	
	if((!IdPeerIguais(peer,tempSearch.peer) ) && (tempSearch.distancia > distancia)) {
		printf("Troquei\n");
		tempSearch.peer = peer;
		tempSearch.distancia = distancia;
	}
	printf("\n");
}

void defineSearch(){
	
	Header requisicao;
	
	// Funcao que retorna o peer escolhido
	#ifdef DEBUG_SEARCH
		char sPeer[TAM_STR_IP + 1];
		IdPeer2StrIp(sPeer, tempSearch.peer);
		fprintf(stdout, "***************************************\n");
		fprintf(stdout, "PEER SELECIONADO PARA BUSCA DO ARQUIVO:\n");
		fprintf(stdout, "----> %s <----\n", sPeer);
		fprintf(stdout, "***************************************\n");
	#endif
	
	
	createHeader(&requisicao, GET, tempSearch.peer);
	requisicao.tamDados = strlen(tempSearch.nomeArquivo);
	
	InsereMensagemResposta(requisicao.numSeq, JOINR);
	distribuiGet(requisicao, tempSearch.nomeArquivo);
	
	inicializaGetResponse(tempSearch.nomeArquivo);
}

/*******
 * FUNCOES DE RECEBIMENTO DE ARQUIVOS
 * *****/

void incrementaPacotesRecebidos()
{
	getResponse.ignoraTimeouts++;
}

void decrementaPacotesRecebidos()
{
	getResponse.ignoraTimeouts--;
}

int retornaPacotesRecebidos()
{
	return getResponse.ignoraTimeouts;
}

void inicializaGetResponse(char *arquivo){
	
	strcpy(getResponse.nomeArquivo, arquivo);
	getResponse.recebendo = true;
	getResponse.ignoraTimeouts = 0;
}

void getNomeArquivo(char *nome){
	
	strcpy(nome, getResponse.nomeArquivo);
}

bool getRecebendo(){
	
	return getResponse.recebendo;
}

void setRecebendo(bool pEstado){
	
	getResponse.recebendo = pEstado;

}

/***
FUNCOES DE DISTRIBUICAO DE MENSAGENS
***/

// Faz o flooding
void distribui( Header& h, char * dadosP )
{
	IdPeer vizinhoRemetente = h.remetente;
	
	#ifdef DEBUG
		char tmpVizinho[TAM_STR_IP + 1];
		puts("Vou distribuir msg");
	#endif
	
	// Distribui MSG para os vizinhos
	for(int i=0; i< MAX_VIZINHOS; i++) {
		if((vizinhos[i].sockfd != -1) && (!IdPeerIguais(vizinhos[i].vizinho, vizinhoRemetente))) 
		{
			#ifdef DEBUG
				IdPeer2StrIp(tmpVizinho, vizinhos[i].vizinho);
				printf("\nEstou distribuindo msg para o vizinho: %s\n", tmpVizinho);
			#endif
			
			h.destino = vizinhos[i].vizinho;
			
			if(!escrevePacote(vizinhos[i].sockfd, h, dadosP))
					puts("Erro no envio de msg");
			
			#ifdef DEBUG
				puts("Distribui - Msg Enviada com sucesso!");
			#endif
		}
	}
}

void distribuiGet(Header& h, char * dadosP)
{
	for(int i=0; i< MAX_VIZINHOS; i++) {
		if((vizinhos[i].sockfd != -1) && (!IdPeerIguais(vizinhos[i].vizinho, h.remetente))) 
		{
			if(!escrevePacote(vizinhos[i].sockfd, h, dadosP))
					puts("Erro no envio de msg (Get)");
		}
	}
}

void encaminha(int peerfd, Header& h, char * dadosP) {	// Encaminha pelo enlace correto
	
	int sockVizinho;
	IdPeer vizinho;
	
	// ***********************************************************
	// IMPORTANTE !!!!!!
	// Descobre o vizinho q me mandou a msg originalmente - o tipo de msg agora eh uma resposta da original !!!!
	// ***********************************************************
	
	#ifdef DEBUG_ROT
		printf("\n\n*** ROTEAMENTO (INICIO) ***\n");
	#endif
	
	if(RetornaRegRoteamento(vizinho, h.destino, converteTipoMsg(h.tipoMsg), h.numSeq)) {			

		#ifdef DEBUG_ROT
			char tmp[TAM_STR_IP + 1];
			IdPeer2StrIp(tmp, vizinho);
			printf("Vizinho escolhido: %s\n", tmp);
			IdPeer2StrIp(tmp, h.remetente);
			printf("Origem: %s\n", tmp);
			IdPeer2StrIp(tmp, h.destino);
			printf("Destinho: %s\n", tmp);
			
			imprimeRoteamento();
		#endif
		
		// Busca o socket na tabela de vizinhos
		if((sockVizinho = buscaSocketVizinho(vizinho)) == -1) {
			if((sockVizinho = encontraTabelaSocketPorIdPeer(vizinho)) == -1)
			{
				#ifdef DEBUG_ROT
					printf("Vizinho NÃO ENCONTRADO!\n");
					printf("*** ROTEAMENTO (FIM) ***\n\n\n");
				#endif
			}
		}
//		puts("***********************************************");
//		printf("Tamanho dos dados q vou enviar: %d -- Tipo: %d \n", h.tamDados, h.tipoMsg);
//		puts("***********************************************");
				
		if(!escrevePacote(sockVizinho, h, dadosP))
			puts("Erro no encaminhamento de msgs");
		
	}

	#ifdef DEBUG_ROT
		printf("*** ROTEAMENTO (FIM) ***\n\n\n");
	#endif
}


// Mensagem

void InicializaTabelaMensagensValidas()
{
	for(int i=0; i<QTD_TIPOS_MSGS_RESP ; i++)
	{
		MsgsValidas[ i ].tipoMsg = -1;
		MsgsValidas[ i ].numSeq = -1;
		MsgsValidas[ i ].valido = false;
	}
}


void InsereMensagemResposta(short numSeq, char tipoMsg)
{
	int i = ConverteTipoMsgParaIndice(tipoMsg);
	
	if(i == -1)	return;
	
	MsgsValidas[ i ].tipoMsg = tipoMsg;
	MsgsValidas[ i ].numSeq = numSeq;
	MsgsValidas[ i ].valido = true;
}

void RemoveMensagemResposta(int tipoMsg)
{
	int i = ConverteTipoMsgParaIndice(tipoMsg);
	
	if(i == -1)	return;
	
	MsgsValidas[ i ].valido = false;
}

bool MensagemEhValida(int tipoMsg, short numSeq)
{
	int i = ConverteTipoMsgParaIndice(tipoMsg);
	
	if(i == -1)	return true; // Caso naum seja uma msg de resposta
	
	if(MsgsValidas[ i ].numSeq == numSeq)
		return MsgsValidas[ i ].valido;
	else
		return false;
}

bool isValida(int tipoMsg){
	
	int i = ConverteTipoMsgParaIndice(tipoMsg);
	
	if(i == -1)	return true; // Caso naum seja uma msg de resposta
	return MsgsValidas[ i ].valido;
	
}



//*************************
// Roteamento.
//*************************

void imprimeRoteamento()
{
	std::list<RegRoteamento>::iterator it;
	RegRoteamento reg;
	char orig[TAM_STR_IP + 1];
	char viz[TAM_STR_IP + 1];
	
	puts("TABELA ROTEAMENTO (INICIO)");
	
	for(it = TabelaRoteamento.begin() ; it!=TabelaRoteamento.end() ; it++) {
		reg = *it;
		
		IdPeer2StrIp(orig, reg.origem);
		IdPeer2StrIp(viz, reg.vizinho);
				
		printf("Origem: %s\tVizinho: %s\tNumSeq: %d\tTipoMsg: %d\n",
				orig, viz, reg.numSeq, reg.tipoMsg );
	}
	
	puts("TABELA ROTEAMENTO (FIM)");
}

bool RetornaRegRoteamento(IdPeer& vizinho, const IdPeer& origem, char tipoMsg, short numSequencia)
{
	std::list<RegRoteamento>::iterator it;
	RegRoteamento reg;

	if(TabelaRoteamento.empty()) return false;
	
	#ifdef DEBUG1
		puts("Comecando a buscar na tabela de roteamento");
	#endif
	
	for(it = TabelaRoteamento.begin() ; it!=TabelaRoteamento.end() ; it++)
	{
		reg = *it;
		if( IdPeerIguais(reg.origem, origem) &&
		     (reg.tipoMsg == tipoMsg) &&
		     (reg.numSeq == numSequencia) )
		     break;
	} 
	
	#ifdef DEBUG1
		puts("Terminando de buscar na tabela de roteamento");
	#endif
	
	if(it == TabelaRoteamento.end())
		return false;
	
	reg =  *it;
	
	vizinho = reg.vizinho;
	
	#ifdef DEBUG1
		puts("Vou retornar o vizinho da tabela de roteamento");
	#endif
		
	return true;
}


bool InsereTabRoteamento(const IdPeer& vizinho, const IdPeer& origem, char tipoMsg, short numSequencia)
{
	RegRoteamento reg;
	IdPeer dummy;
			
	if(!RetornaRegRoteamento(dummy, origem, tipoMsg,numSequencia))
	{
		reg.tipoMsg = tipoMsg;
		reg.origem = origem;
		reg.numSeq = numSequencia;
		reg.vizinho = vizinho;
		
		TabelaRoteamento.push_back(reg);
		
		return true;
	}
	
	return false;
}


bool isReceived( Header& h ){
	
	IdPeer dummy;
	
	#ifdef DEBUG_ROT
		puts("Vai tentar descobrir se o registro estah na tabela de roteamento");
	#endif
	
	return RetornaRegRoteamento(dummy, h.remetente, h.tipoMsg, h.numSeq);
}



/*************************
	Funcoes utilitarias
 *************************/

/* ------------------------------ ToLower ------------------------------ */

char *toLower(char *str)
{
	register unsigned short i;

	for(i=0; i < strlen(str); i++) str[i]=tolower(str[i]);

	return str;
}


bool IdPeerIguais(const IdPeer& id1, const IdPeer& id2)
{
	if( (id1.oc1 == id2.oc1) &&
	     (id1.oc2 == id2.oc2) &&
	     (id1.oc3 == id2.oc3) &&
	     (id1.oc4 == id2.oc4) &&
	     (id1.porta == id2.porta) )
	     	return true;
	
	return false;
}


/******************************************************
	Tabela de Clientes
	Guarda os sockets conectados a aplicação.
 ******************************************************/

bool existeTabelaSocket(int sock)
{
	std::list<RegSocket>::iterator it;
	RegSocket reg;
	
	for(it = TabelaSocket.begin() ; it!=TabelaSocket.end() ; it++) {
		reg = *it;
		if(reg.sockfd == sock)
			return true;
	}
	
	return false;
}
 
bool insereTabelaSocket(int sock)
{
	std::list<RegSocket>::iterator it;
	RegSocket reg;
	
	for(it = TabelaSocket.begin() ; it!=TabelaSocket.end() ; it++) {
		reg = *it;
		if(reg.sockfd == sock)
			return false;
	}
	
	reg.sockfd = sock;
	reg.peer.oc1 = 0;
	reg.peer.oc2 = 0;
	reg.peer.oc3 = 0;
	reg.peer.oc4 = 0;
	reg.peer.porta = 0;
	
	TabelaSocket.push_back(reg);
	
	return true;
}

bool removeTabelaSocket(int sock)
{
	std::list<RegSocket>::iterator it;
	RegSocket reg;
	
	for(it = TabelaSocket.begin() ; it!=TabelaSocket.end() ; it++) {
		reg = *it;
		if(reg.sockfd == sock) {
			TabelaSocket.erase(it);
			return true;
		}
	}
	
	return false;
}

bool atualizaIdPeerTabelaSocket(int sockP, IdPeer peerP)
{
	std::list<RegSocket>::iterator it;
	RegSocket reg;
	
	for(it = TabelaSocket.begin() ; it!=TabelaSocket.end() ; it++) {
		reg = *it;
		if((reg.sockfd == sockP) && (reg.peer.porta == 0)) {
			reg.peer = peerP;
			*it = reg;
			return true;
		}
	}	
	return false;
}

int encontraTabelaSocketPorRSet(fd_set rset)
{
	std::list<RegSocket>::iterator it;
	RegSocket reg;
	
	if(TabelaSocket.empty()) return -1;
	
	for(it = TabelaSocket.begin() ; it!=TabelaSocket.end() ; it++) {
		reg = *it;
		if(FD_ISSET(reg.sockfd, &rset))
		{
			return reg.sockfd;
		}
	}
	
	return -1;
}

int encontraTabelaSocketPorIdPeer(IdPeer& peerP)
{
	std::list<RegSocket>::iterator it;
	RegSocket reg;
	
	if(TabelaSocket.empty()) return -1;
	
	for(it = TabelaSocket.begin() ; it!=TabelaSocket.end() ; it++) {
		reg = *it;
		if(IdPeerIguais(reg.peer, peerP)) {
			return reg.sockfd;
		}
	}
	
	return -1;
}

bool atualizaTabelaSocketPorIdPeer(IdPeer& peerP, int pSockfd)
{
	std::list<RegSocket>::iterator it;
	RegSocket reg;
	
	if(TabelaSocket.empty()) return -1;
	
	for(it = TabelaSocket.begin() ; it!=TabelaSocket.end() ; it++) {
		reg = *it;
		if(IdPeerIguais(reg.peer, peerP)) {
			reg.sockfd = pSockfd;
			return true;
		}
	}
	
	return false;
}

void imprimeTabelaSocket()
{
	std::list<RegSocket>::iterator it;
	RegSocket reg;
	char sIp[TAM_STR_IP + 1];
	
	printf("Tabela de Socket:\n");
	
	for(it = TabelaSocket.begin() ; it!=TabelaSocket.end() ; it++) {
		reg = *it;
		
		IdPeer2StrIp(sIp, reg.peer);
		
		printf("Vizinho: %s\tSocket: %d\n", sIp, reg.sockfd);
	}
}


/* ------------------------------ PrintHelp ------------------------------ */
void printHelp()
{
	printf("\nComandos validos: \n - entrada\n - get arquivo [nome do arquivo]\n - saida\n\n");
}


int ConverteTipoMsgParaIndice(int tipoMsg)
{
	if(tipoMsg == JOINR)
		return 0;
	else if(tipoMsg == SEARCHR)
		return 1;
	else if(tipoMsg == GETR)
		return 2;

	return -1;
}

char converteTipoMsg(char tipoMsg){
	
	if(tipoMsg == JOINR)
		return JOIN;
	else if(tipoMsg == SEARCHR)
		return SEARCH;
	else if(tipoMsg == GETR)
		return GET;

	return -1;
}


void verificaTabelaVizinhosSocketsDesconhecidos()
{
	int sock;
	
	for(int i = 0; i < MAX_VIZINHOS; i++) {
		if(vizinhos[i].sockfd != -1) {
			if(vizinhos[i].sockfd == SOCKET_DESCONHECIDO) {
				if((sock = encontraTabelaSocketPorIdPeer(vizinhos[i].vizinho)) != -1) {
					vizinhos[i].sockfd = sock;
				}
				else {
					// TODO -- o cara pode ter caido.
					vizinhos[i].sockfd = abreConexao(vizinhos[i].vizinho);
					if(!insereTabelaSocket(vizinhos[i].sockfd))
						puts("Socket jah estah na tabela de sockets");
					
					//if(!atualizaIdPeerTabelaSocket(vizinhos[i].sockfd, vizinhos[i].vizinho))
					//	puts("Erro ao atualizar tabela de sockets - funcao verificaTabelaVizinhosSocketsDesconhecidos()");
				}
			}
		}
	}
}
