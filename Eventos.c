/* UFRJ - DCC - Teleprocessamento e Redes - 2004/2
	Fabio Pereira Sarmento - DRE:
	Gustavo Zopelari Nasu - DRE: 098212015
	Leonardo Freitas Gomes - DRE: 098211336
	Ricardo Chiganer Lilenbaum - DRE: 101133019 */

#include "Principal.h"
#include "Eventos.h"
#include "Estruturas.h"

extern TDadosAplicacao DadosAplicacao;
extern RegVizinho vizinhos[MAX_VIZINHOS];



/* ------------------------------ onEntrada ------------------------------ */

void onEntrada(){

	int ret;
	Header header;
	IdPeer dest = DadosAplicacao.noPrincipalIp ;  // Destino = Ip do noh onde conectei

	createHeader(&header, JOIN, dest);
	header.tamDados = 0;

	ret = u_send( DadosAplicacao.sockPrincipalConectado, &header, (size_t) sizeof(header) );
	if(ret == -1) puts("Erro no envio de join msg");

	if(ret >= 0){
		puts("Join Msg enviada com sucesso");
		printf("Bytes lidos: %d\n", sizeof(header) );
		printf("Bytes enviados: %d\n", ret);
	}
	
	/* Evento que originou o registro na tabela foi uma JOIN MSG, mas o evento que estah sendo monitorado pelo timer eh a JOIN RESPONSE MSG */
	InsereMensagemResposta(header.numSeq, JOINR);
	SetTimer( DadosAplicacao.intervaloTimeout, TIMER_JOINR, 0 );

}

/* ------------------------------ onSaida ------------------------------ */

void onSaida(){
	printf("\nObrigado por utilizar o p2p!\n");
	/* TODO
		- Fechar todos os sockets que possam estar abertos
		- Encerrar as threads
	*/
	exit(0);
}

/* ------------------------------ onGetArquivo ------------------------------ */

void onGetArquivo(char *str){

	char nomeDoArquivo[MAX_NOME_ARQ];
	Header header;
	IdPeer dest;
			
	scanf("%s", str);
	
	inicializaSearch();
	
	//Checa se os tokens estao ok
	if(strcmp(str, "arquivo"))
	{
		printHelp();
		return;
	}

	scanf("%s", str);	

	strcpy(nomeDoArquivo,str);
	
	printf("Nome do arquivo: (%s)\n", nomeDoArquivo);
	
	// Checa se ha algum token depois do nome do arquivo
	if(  (str = strtok(NULL, " ") ) != NULL){
		printHelp();
		return;
	}
	
	if(isValida(SEARCHR) || isValida(GETR)) {
		printf("******************************************************\n");
		printf("Buscando arquivo... \nAguarde para realizar nova busca\n");
		printf("******************************************************\n");
		return;
	}
	
	createHeader(&header, SEARCH, dest);
	header.tamDados = strlen(nomeDoArquivo);
	
	#ifdef DEBUG_SEARCH
		printf("Tamanho do nome do arquivo: %d\n", header.tamDados);
	#endif
	
	distribui(header, nomeDoArquivo);
	setNomeArquivoSearch(nomeDoArquivo);
		
	#ifdef DEBUG_SEARCH
		printf("SEARCH distribuida");
	#endif
	
	InsereMensagemResposta(header.numSeq, SEARCHR);
	SetTimer(DadosAplicacao.intervaloTimeout, TIMER_SEARCHR, 0);
}
	

void onImprimeTabelaVizinho()
{
	imprimeTabelaVizinhos();
}

void onRoteamento(){
	imprimeRoteamento();
}


/* ------------------------------ trataJoin ------------------------------ */

void trataJoin(int sockfd, Header& h, char * dadosP){

	Header resposta;
	char dados[1];
	
//	#ifdef DEBUG
		char tmp[TAM_STR_IP + 1];
		IdPeer2StrIp(tmp, h.remetente);
		printf("\n********** RECEBIMENTO DE JOIN *************\n");
		printf("Remetente: %s\n", tmp);
		printf("NumSeq: %d\n", h.numSeq);
		printf("********************************************\n\n");		
//	#endif
	
	
	puts("Inseriu na tabela de roteamento");

	// Se ha espaco na tabela de vizinhos, adiciona o vizinho e envia JOINR com distancia
	//if( insereVizinho(origem, sockfd) ){
	if(contaVizinhos() < MAX_VIZINHOS){
		
		createHeader( &resposta, JOINR, h.remetente );
		resposta.numSeq = h.numSeq;
		
		dados[0] = calcDistancia( &DadosAplicacao.meuIp, &h.remetente );
		
		printf("\n\n\n**************\nDISTANCIA JOINR: %d\n*****************\n\n\n", dados[0]);
		resposta.tamDados = sizeof( unsigned char );
		printf("Tamanho dos Dados da JOINR: %d\n\n", resposta.tamDados);
		
		if(!escrevePacote(sockfd, resposta, dados))
			puts("Erro ao enviar JOINR MSG");

	//		if ( ( posicao = buscaPosicaoVizinho(h.remetente) ) != -1)
	//		SetTimer(DadosAplicacao.intervaloHelloR, TIMER_HELLOR, posicao);
	}
	
	//***********************************************************
		distribui(h, NULL);
	//***********************************************************
}

/* ------------------------------ trataJoinResponse ------------------------------ */

void trataJoinResponse(int sockfd, Header& h, char * dadosP)
{
	char *distancia;
	int duplicado;
	IdPeer vizinho = h.remetente;
	
	#ifdef DEBUG_JOINR
		printf("\n\n*** JOIN RESPONSE (INICIO) ***\n");
	#endif
	
	if( (distancia = (char *) malloc(h.tamDados+1) ) == NULL){
		puts("Erro de memoria");
		exit(1);
	}
	
	#ifdef DEBUG
		printf("Tamanho dos dados que vou ler: %d\n",  h.tamDados);
	#endif
	
	strncpy(distancia, dadosP, h.tamDados);
	distancia[h.tamDados] = '\0';
	
	#ifdef DEBUG
		printf("Tamanho dos dados recebidos: %d\n", ret );
		printf("Dados recebidos: %s\n", distancia);
	#endif
	
	//Escolhe os vizinhos
	if ( (duplicado = buscaSocketVizinho(vizinho)) == -1 ){ 	// Se naum estah na tabela de vizinhos
		
		escolheVizinhos(vizinho, atoi(distancia), SOCKET_DESCONHECIDO);
	}
	
	#ifdef DEBUG_JOINR
		printf("Vizinhos temporarios: \n");
		imprimeTabelaVizinhosTemp();
		printf("*** JOIN RESPONSE (FIM) ***\n\n\n");
	#endif
}

/* ------------------------------ trataHello ------------------------------ */

void trataHello(int sockfd, Header& h, char * dadosP)
{
	int distancia = 1,
		duplicado;
	
	#ifdef DEBUG_HELLO
		char tmp[TAM_STR_IP + 1];
		IdPeer2StrIp(tmp, h.remetente);
		printf("\n\n*** RECEBI HELLO (INICIO) ***\n");
		printf("Vizinho: %s\n", tmp);
	#endif
	
	
	if ( (duplicado = buscaSocketVizinho(h.remetente)) == -1 ) { 	// Se naum estah na tabela de vizinhos
		escolheVizinhos(h.remetente, distancia, sockfd);
		defineVizinhos();
		InicializaTabelaTempVizinhos();
	}
	
	#ifdef DEBUG_HELLO
		imprimeTabelaVizinhos();
		printf("*** RECEBI HELLO (FIM) ***\n\n\n");
	#endif
}

void trataSearch(int sockfd, Header& h, char * nomeArquivo)
{
	Header resposta;
	char dados[1];
	FILE *arquivo;
	char sNomeArquivo[MAX_NOME_ARQ];
	
	strncpy(sNomeArquivo, nomeArquivo, h.tamDados);
	sNomeArquivo[h.tamDados] = '\0';
	
	#ifdef DEBUG_SEARCH
		char tmp[TAM_STR_IP + 1];
		IdPeer2StrIp(tmp, h.remetente);
		printf("\n********** RECEBIMENTO DE SEARCH *************\n");
		printf("Remetente: %s\n", tmp);
		printf("NumSeq: %d\n", h.numSeq);
		printf("********************************************\n\n");		
		
		printf("\n\n\n\n********** VOU CHECAR SE TENHO O ARQUIVO !!!! *************\n");
		printf("\n\n\n\n********** NOME DO ARQUIVO: (%s)", sNomeArquivo);
		
	#endif
	
	arquivo = fopen(sNomeArquivo, "r");
	
	if(arquivo) {
		
		printf("\n\n\n\n********** ARQUIVO ENCONTRADO !!!! *************\n");
		
		createHeader(&resposta, SEARCHR, h.remetente);
		resposta.numSeq = h.numSeq;
		
		dados[0] = calcDistancia(&DadosAplicacao.meuIp, &h.remetente );
		resposta.tamDados = sizeof(unsigned char);
		
		if(!escrevePacote(sockfd, resposta, dados))
			puts("Erro ao enviar SEARCHR MSG");
		
		#ifdef DEBUG_SEARCH
			printf("\n********** RESPONDEU SEARCH *************\n");
		#endif
		
		fclose(arquivo);
	}
	
	//***********************************************************
	distribui(h, nomeArquivo);
	//***********************************************************
	
	#ifdef DEBUG_SEARCH
		printf("\n********** DISTRIBUIU SEARCH *************\n");
	#endif
}


void trataSearchResponse(int sockfd, Header& h, char * dadosP)
{
	char *distancia;
	
	if( (distancia = (char *) malloc(h.tamDados+1) ) == NULL){
		puts("Erro de memoria");
		exit(1);
	}
	
	strncpy(distancia, dadosP, h.tamDados);
	distancia[h.tamDados] = '\0';
	
	escolheSearch( h.remetente, distancia[0] );

	SetTimer( DadosAplicacao.intervaloTimeout, TIMER_GETR, 0 );	
	
	#ifdef DEBUG_SEARCH
		printf("\n********** RECEBEU SEARCH RESPONSE *************\n");
	#endif
}


/*
 * 
 * TRATA REQUISICOES DE ARQUIVOS
 * 
 * */

void trataGet(int sockfd, Header& h, char *nomeArquivo ){

	int falha;
	FILE *arquivo;
	char sNomeArquivo[MAX_NOME_ARQ];
	TArgumentoGet *args;	//Argumentos da funcao passada para a thread
	pthread_t idThread;		//id da thread criada
	
	strncpy(sNomeArquivo, nomeArquivo, h.tamDados);
	sNomeArquivo[h.tamDados] = '\0';
	
	arquivo = fopen(sNomeArquivo, "r");
	
	printf("\n\nInicio 'TRATA GET'\n");
	
	if(arquivo) {
			
		printf("Enviando arquivo\n");
			
			/* aloca estrutura a ser passada para a thread e atribui valor as variavies */
		if (  (args = (TArgumentoGet *)malloc(sizeof(TArgumentoGet)))==NULL  ) {
			printf("Erro de memoria!\n");
			exit(1);
		}
		strcpy(args->nomeArquivo, sNomeArquivo);
		args->sockfd = sockfd;
		args->header = h;
			
			
		falha = pthread_create(&idThread, NULL, &enviaArquivo, (void *) args);
			
		if(falha){
			printf("Erro ao criar thread para atender requisicao de arquivo\n");
			free(args);
		}
		
		fclose(arquivo);
	}

	printf("Fim 'TRATA GET'\n\n\n");
	
//	free(args);
}

void *enviaArquivo(void *args){
	
	FILE *arquivo;
	TArgumentoGet *argumentos = (TArgumentoGet *) args;
	Header resposta, requisicao;
	char bloco[TAM_BLOCO];
	int tamDados;
	
	requisicao = argumentos->header;
	
	// Monta o cabecalho da mensagem
	createHeader(&resposta, GETR, requisicao.remetente);
	resposta.numSeq = requisicao.numSeq;
	
	arquivo = fopen(argumentos->nomeArquivo, "r");
	
	if(arquivo) {
		
		printf("\n**** ENVIANDO ARQUIVO *****\n");
		
		while(!feof(arquivo)){
			
			tamDados = fread(bloco, (size_t) 1, (size_t) TAM_BLOCO, arquivo);
			resposta.tamDados = tamDados;
			if(!escrevePacote(argumentos->sockfd, resposta, bloco))
				printf("Erro ao enviar fragmento do arquivo");
			
			usleep(100);
		}
		
		// Envia pacote com ZERO BYTES sinalizando que terminou de enviar o arquivo
		resposta.tamDados = 0;
		escrevePacote(argumentos->sockfd, resposta, NULL);
		
	}
	fclose(arquivo);
	return NULL;
}

void trataGetResponse(int sockfd, Header& h, char *pDados ) {
	
	FILE *arquivo;
	char nomeArquivo[MAX_NOME_ARQ];
	int tamDados;

	
	if(getRecebendo()){
		
		if(h.tamDados == 0) {
			setRecebendo(false);
			printf("Download completo.\n");
			RemoveMensagemResposta(GETR);
			return;
		}
		
		getNomeArquivo(nomeArquivo);
		arquivo = fopen(nomeArquivo, "a+");

		if(arquivo){
			
			if((tamDados = fwrite(pDados, (size_t) 1, (size_t) h.tamDados, arquivo)) != h.tamDados) {
					setRecebendo(false);
					printf("Download interrompido.\n");
					fclose(arquivo);
					remove(nomeArquivo);
					return;
			}
			else {
				incrementaPacotesRecebidos();
			}
						
			fclose(arquivo);
		}
		
		SetTimer( DadosAplicacao.intervaloTimeout, TIMER_GETR, 0 );
	}
}


/*************************
EVENTOS DE TIMEOUT
*************************/

void trataTimerHello(){
	int i;
	Header h;
	// Checa 'Hello's' recebidos.
	// TODO
	
	// Envia 'Hello's'.
	for(i=0 ; i<MAX_VIZINHOS ; i++)	{
		if(vizinhos[i].sockfd > 0) {
			createHeader(&h, HELLO, vizinhos[i].vizinho);
			h.tamDados = 0;
			if(!escrevePacote(vizinhos[i].sockfd, h, NULL))
				puts("Erro no envio de HELLO");
		}
	}
	
	SetTimer( DadosAplicacao.intervaloHello, TIMER_HELLO, 0 );
}


void trataTimerJoinR()
{
	int qtdVizinhos;
	
	RemoveMensagemResposta(JOINR);
	defineVizinhos();
	
	verificaTabelaVizinhosSocketsDesconhecidos();
		
	
	qtdVizinhos = contaVizinhos();
	
	#ifdef DEBUG
		printf("\n\n");
		puts("*****************************************");
		puts("TABELA DE VIZINHOS");
		puts("*****************************************");
		onImprimeTabelaVizinho();
		printf("\n\n");
		
	#endif
		
	InicializaTabelaTempVizinhos();
	
	if (qtdVizinhos > 0) 
		SetTimer(DadosAplicacao.intervaloHello, TIMER_HELLO, 0);
	
	if ( qtdVizinhos < MIN_VIZINHOS ){
		// Disparo novo evento de entrada para conseguir mais vizinhos
		#ifdef DEBUG_VIZINHOS
			puts("Vou tentar encontrar novos vizinhos");
		#endif
		onEntrada();
	}
}

void trataTimerSearchR()
{
	RemoveMensagemResposta(SEARCHR);
	
	#ifdef DEBUG_SEARCH
		puts("******************************");
		puts("DEFININDO SEARCH");
		puts("******************************");
	#endif		
	
	defineSearch();
		
	// Aqui tenho que disparar a getmsg
}


void trataTimerGetR()
{
	char nomeArquivo[MAX_NOME_ARQ];

	decrementaPacotesRecebidos();
	
	if(getRecebendo()) {
		if(retornaPacotesRecebidos() == 0)
		{
			getNomeArquivo(nomeArquivo);
			setRecebendo(false);
			printf("Download interrompido.\n");
			remove(nomeArquivo);
			RemoveMensagemResposta(GETR);
		}
	}
}


void disparaHello()
{
	Header hello;
	IdPeer destino;

	// Destino serah setado pela funcao distribui()	
	memset( &destino, 0, sizeof(IdPeer));
	
	createHeader(&hello, HELLO, destino);
	hello.tamDados = 0;
	#ifdef DEBUG
		printf("\n\n");
		puts("*****************************************");
		puts("Vou distribuir hello");
		puts("*****************************************");
		printf("\n");
	#endif
	
	distribui(hello, NULL);
}
