#include "Principal.h"
#include "Peer.h"

extern TDadosAplicacao DadosAplicacao;

fd_set rset;			// Conjunto de descritores q vou monitorar para read (posso criar um cjto de descritores para write tb - wset).
fd_set allset;			// Conjunto total de descritores.
int	maxfd = 0;


int abreConexao(IdPeer& peer)
{
	char sIp[TAM_STR_IP+1];
	int sockfd;
	
	printf("****Entrei****\n");
	
	IdPeer2StrIp(sIp, peer);
	
	printf("Vou tentar estabelecer nova conexao com o peer: %s\n", sIp);
	sockfd = u_connect(peer.porta, sIp);
	printf("Socket: %d\n", sockfd);
	
	FD_SET(sockfd, &allset); 	// Seta o bit para o novo descritor no set de descritores q estao sendo tratados por "select"
	
	if(sockfd > maxfd)
		maxfd = sockfd;
	
	printf("****Saindo****\n");
		
	return sockfd;
}

// Fun�o: ReadCmdLine
// @retor: void - Nada.
// Descri�o: Le e processa comando do teclado.

void readCmdLine()
{
	char readline[MAX_CMD_ENTRADA];

	scanf("%s", readline);
			
	strcpy(readline, toLower(readline));
			
	if(DadosAplicacao.sockPrincipalConectado == -1){
		puts("Voce ainda nao esta conectado... Aguarde...");
		return;
	}
	
	switch(readline[0])
	{
		//	Entrada.
		case 'e':		
			onEntrada();
			break;
		
		//	Get Arquivo.
		case 'g':
			onGetArquivo(readline);
			break;
		
		//	Saida.
		case 's':
			onSaida();
			break;
		
		//	Imprime estatisticas.
		case 'i':
			
			break;

		case 'r':
			onRoteamento();
			break;

		case 'v':
			onImprimeTabelaVizinho();
			break;
			
		case 't':
			imprimeTabelaSocket();
			break;
		
		default:
			printHelp();
	}
}


/* ------------------------------ trataTimer ------------------------------ */

void trataTimer()
{
	switch( RetornaTipoTimer()  ){
		case TIMER_JOINR:
			trataTimerJoinR();
			break;
		case TIMER_HELLO:
			trataTimerHello();
			break;
		case TIMER_HELLOR:
			// Condicionado ao recebimento de hello - tenho q implementar trataHello
			break;
		case TIMER_SEARCHR:
			trataTimerSearchR();
			break;
		case TIMER_GETR:
			trataTimerGetR();
			break;
		default:
			puts("Erro no monitoramento de timeouts");
	}
	
	TimerTratado();
}


void trataRecebimento(int peerfd)
{
	//int ret;
	Header h;
	IdPeer vizinho;
	char dadosPacote[TAM_MAX_SEG];
	
//	printf("Le Pacote\n");
	lePacote(peerfd, h, dadosPacote);
	
//	printf("Tipo de msg: %d\n\n", h.tipoMsg);
	
	#ifdef DEBUG
		char tmp[TAM_STR_IP + 1];
		IdPeer2StrIp(tmp, h.remetente);
		printf("****** MENSAGEM LIDA PELO SOCKET ******\n");
		printf("Origem de mensagem: %s\n", tmp);
		printf("Tipo de mensagem: %d\n", h.tipoMsg);
		printf("Tamanho dos dados: %d\n", h.tamDados);
		printf("Numero de sequencia: %d\n", h.numSeq);
		printf("***************************************\n");
	#endif
	
	if(!MensagemEhValida(h.tipoMsg, h.numSeq) )
	{
		printf("\n\n*********************************************************************\n");					
		printf("****************Mensagem descartada pois o timer expirou!*************\n");					
		printf("*********************************************************************\n");					
		return;
	} 
	
	// Verifico se a mensagem já foi recebida.
	if(isReceived(h)){
//		#ifdef DEBUG1
			printf("\nMsg já foi recebida (numSeq = %d\tTipo: %d)\n",h.numSeq, h.tipoMsg);
//		#endif
		return;
		
	}
	
	atualizaIdPeerTabelaSocket(peerfd, h.remetente);
	
	// Verifico se a mensagem deve ser roteada.
	char r[TAM_STR_IP+1];
	char d[TAM_STR_IP+1];
	
	IdPeer2StrIp(r, h.remetente);
	IdPeer2StrIp(d, h.destino);
	
//	if(h.tipoMsg == GET || h.tipoMsg == GETR)
//		printf("\nRem: %s\tDest.: %s\tNumSeq: %d\tTipo: %d\n\n",r, d, h.numSeq, h.tipoMsg);
	
	
	if(!IdPeerIguais(h.destino, DadosAplicacao.meuIp)){
		
		if(h.tipoMsg == GET){
			if(!buscaIdPeerVizinho(peerfd, vizinho)){
				vizinho = h.remetente;
			}
			InsereTabRoteamento(vizinho, h.remetente, h.tipoMsg, h.numSeq);
			distribuiGet(h, dadosPacote);
		}
		else
		{
			encaminha(peerfd, h, dadosPacote);
		}
		return;
	}

	// Insere a mensagem na tabela de roteamento.
	if(ehMensagemRoteada(h.tipoMsg))
	{
		if(!buscaIdPeerVizinho(peerfd, vizinho)){
			vizinho = h.remetente;
		}
		
		#ifdef DEBUG_ROT
			char tmp[TAM_STR_IP + 1];
			IdPeer2StrIp(tmp, h.remetente);
			printf("\n\nVIZINHO ENCONTRADO: %s\n\n", tmp);
		#endif
		
		InsereTabRoteamento(vizinho, h.remetente, h.tipoMsg, h.numSeq);
	}
	
//	printf("\nDEBUG\n");
//	printf("switch\n");
	
	// Tratamento da mensagem.
	switch( h.tipoMsg ){

		case HELLO:
			
			#ifdef DEBUG_HELLO
				puts("CHAMOU HELLO!");
			#endif
			trataHello(peerfd, h, dadosPacote);
			break;

		case JOIN:
			trataJoin(peerfd, h, dadosPacote);
			break;

		case JOINR:
			trataJoinResponse(peerfd, h, dadosPacote);
			break;

		case SEARCH:
			trataSearch(peerfd, h, dadosPacote);
			break;

		case SEARCHR:
			trataSearchResponse(peerfd, h, dadosPacote);
			break;

		case GET:
			trataGet(peerfd, h, dadosPacote);
			break;

		case GETR:
			trataGetResponse(peerfd, h, dadosPacote);
			break;

		default:
			puts("\nERRO: Ocorreu algum erro no cabeçalho da mensagem.\n");
			exit(1);
	}
}


// Fun�o: SelecionaDescritores
// @param: void * -
// @retor: void * -
// Descri�o: Executa o loop da entrada de eventos.

void * SelecionaDescritores(void *args) {
	
	// Indica se o socket com o nó principal esta ativo.
	bool flagSockConectado = false;

	// Sockets a serem manipulados.
	int sockfd = -1;
	int accfd = -1;
	int listenfd = -1;
	
	// Porta e ip do servidor principal.
	int portaConnect;
	char servidor[TAM_STR_IP + 1];
	
	// Ponteiro p/ argumentos.
	TpArgumento * argumentos;
	
	char maquina[MAX_CANON];
	
	//Variaveis necessarias para o uso de "select".
	//int	i;
	//int	maxi;
	int	nready;
	int	peerfd;			//Armazena temporariamente o descritor que estah sendo checado.

	// Inicializa variaveis.
	
	argumentos = (TpArgumento *)args;
	
	listenfd = argumentos->listenfd;
	portaConnect = argumentos->portaConnect; // Se for -1 eh pq eh um noh principal
	strcpy(servidor, argumentos->ipConnect); // Se for NULL eh pq eh um noh principal
	
	maxfd = listenfd;			// Valor inteiro do maior descritor utilizado. Inicialmente soh temos o listenfd
	//maxi = -1;
	
	FD_ZERO(&allset);		// Zera todos os bits do set de descritores que serao checados
	FD_SET(listenfd, &allset);	// Seta o bit do listenfd, ou seja, coloca listenfd no set de descritores q sera usado por "select"
	
	FD_SET(fileno(stdin), &allset);
	if(fileno(stdin) > maxfd)
		maxfd = fileno(stdin);	
	
	if(!DadosAplicacao.principal) {
	
		puts("Entrou como naum principal");
		
		while((sockfd = u_connect(portaConnect, servidor)) == -1) {
			sleep(1);
			puts("Tentando conectar no peer principal...");
		}
		
		DadosAplicacao.sockPrincipalConectado = sockfd;
		if(sockfd >= 0) {
				
				FD_SET(sockfd, &allset);		// Utiliza-se esse socket para falar com o noh principal	
				if(sockfd > maxfd)
					maxfd = sockfd;
				
				insereTabelaSocket(sockfd);
					
				flagSockConectado = true;
		}
		
	}
	
	// 'zSeta' os TIMER's.
	SetTimer( DadosAplicacao.intervaloHello, TIMER_HELLO, 0 );
	
	for(;;) {
		
		#ifdef DEBUG
		fprintf(stdout, "\nInicio do FOR\n");
		fprintf(stdout, "SockPrincipal: %d\n\n\n", DadosAplicacao.sockPrincipalConectado);
		#endif
		
		if(DadosAplicacao.principal && (DadosAplicacao.sockPrincipalConectado != -1) && (!flagSockConectado)) {
			
			sockfd = DadosAplicacao.sockPrincipalConectado;	
			
			if(sockfd >= 0) {
				
				FD_SET(sockfd, &allset);		// Utiliza-se esse socket para falar com o noh principal	
				if(sockfd > maxfd)
					maxfd = sockfd;
				
				insereTabelaSocket(sockfd);
					
				flagSockConectado = true;
			}
		}
		
		// Seta o conjunto de descritores que serah checado para eventos do tipo "read"
		rset = allset;
		
		#ifdef DEBUG
		if(RetornaTimer() == NULL)	fprintf(stdout, "Timer NULL\n");
		else	imprimeTimer();
		#endif
		
		//printf("\nANTES DO SELECT:\n");
		//printf("nready: %d\terrno: %d(%d)\n\n", nready, errno, EINTR);
		
		// Retorna o numero de sockets prontos ou zero caso seja um evento de timeout.
		while(((nready = select(maxfd + 1, &rset, NULL, NULL, RetornaTimer()) ) == -1) && (errno == EINTR));
		
		//printf("\n-----> SELECT <-----\n");
		
		// Caso select retorne -1 e o tipo de erro naum seja interrupcao
		if( nready == -1 ) {
			printf("\n-----> ERRO SELECT <-----\n");
			puts(strerror(errno));
			exit(1);
		}
		
		//*********************************
		// TRATA EVENTO DE TIMEOUT
		//*********************************
		
		if(nready == 0) {
//			printf("\n-----> TIMER <-----\n");
			#ifdef DEBUG
				puts("Vou chamar o trataTimer");
			#endif
			
			trataTimer();
		}
		
		//*********************************
		// TRATA PEDIDOS DE CONEXOES
		//*********************************
		
		if(FD_ISSET(listenfd, &rset))
		{
			printf("\n-----> CONEXAO <-----\n");
			//fprintf(stdout, "*** Listen ***\n");
			if((accfd = u_accept(listenfd, maquina)) >= 0)
			{
				puts("Conectou");
				insereTabelaSocket(accfd);
			}
			else{
					// TODO  Accept deu erro - TRATAR
			}
			
			FD_SET(accfd, &allset); 	// Seta o bit para o novo descritor no set de descritores q estao sendo tratados por "select"
			
			if(accfd > maxfd)
				maxfd = accfd;  		// Variavel q sera utilizada na chamada a "select"
			
			if(--nready <= 0)		// Caso o unico descritor setado for o listenfd
				continue;				// Nao ha mais descritores pendentes de serem tratados
		}
		
		
		//**********************************
		// TRATA EVENTOS DE TECLADO
		//**********************************
		
		if( FD_ISSET( fileno(stdin), &rset ) ) {
			printf("\n-----> TECLADO <-----\n");
			
			fprintf(stdout, "*** Teclado ***\n");
			readCmdLine();
		}
		
		//***********************************
		// TRATA RECEBIMENTO DE MENSAGENS
		//***********************************
		
		if((peerfd = encontraTabelaSocketPorRSet(rset)) != -1) {

			//#ifdef DEBUG
	//			printf("\n-----> MENSAGEM <-----\n");
		//		printf("Socket: %d\n", peerfd);
			//#endif
									
			trataRecebimento(peerfd);
			
			if (--nready <= 0 )
				continue;
		}
	}
}
