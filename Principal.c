#include "Principal.h"


TDadosAplicacao DadosAplicacao;


void escolheNoPrincipal(IdPeer &peer1, char *ip1, char *ip2, char *ip3){
	
	int i;
	char* escolha[3] = {ip1, ip2, ip3};

	if(ip3 == NULL)
		i = (time(NULL) % 2);
	else
		i = (time(NULL) % 3);	
	
	printf("Ip Escolhido: %s\n\n", escolha[i]);
	
	peer1 = string2IdPeer(escolha[i]);

}


void * ConectaSocketPrincipal(void * param)
{
	char noPrincipalIp[TAM_STR_IP + 1];
	
	IdPeer2StrIp(noPrincipalIp, DadosAplicacao.noPrincipalIp);
	puts("Tentando conectar em outro peer principal... Aguarde!");
	
	while((DadosAplicacao.sockPrincipalConectado = u_connect(DadosAplicacao.noPrincipalIp.porta, noPrincipalIp)) == -1) {
		sleep(2);	// Aguarda 5s antes de tentar conectar novamente.
		puts("Tentando conectar em outro peer principal...");
	}
	
	puts("Consegui conectar no peer principal");
	
	return NULL;
}

/*****************
	Funcao: main
 *****************/

int main(int argc, char *argv[]) {

	char servidor[TAM_STR_IP + 1];
		
	int falha;
	TpArgumento *args;		//Argumentos da funcao passada para a thread
	pthread_t idThread;		//id da thread criada


	// Checa parametros da aplicacao.
	
	if(argc < 7 || argc > 8 ) {
		printf("Utilizacao: %s hello timeout live ip_desse_noh:porta [ip_dos_nohs_principais:porta]\n",argv[0]);
		exit(1);
	}
	
	DadosAplicacao.principal = (argc < 7) ? true : false;
	
	// Inicializa variaveis da aplicacao.
	
	DadosAplicacao.intervaloHello = atoi(argv[1]);
	DadosAplicacao.intervaloHelloR = 2 * DadosAplicacao.intervaloHello;
	DadosAplicacao.intervaloTimeout = atoi(argv[2]);
	DadosAplicacao.tempoLive = atoi(argv[3]);
	DadosAplicacao.meuIp = string2IdPeer(argv[4]);
	strcpy(DadosAplicacao.meuIpString, argv[4]);

	if(DadosAplicacao.principal)
		escolheNoPrincipal( DadosAplicacao.noPrincipalIp, argv[5], argv[6], NULL );
	else 
		escolheNoPrincipal( DadosAplicacao.noPrincipalIp, argv[5], argv[6], argv[7] );		
	

	IdPeer2StrIp(servidor, DadosAplicacao.noPrincipalIp);
	
	printf("**********************************\n");
	printf("Vou me conectar no ip: %s\n\n", servidor);
	printf("**********************************\n");
	
		
	InicializaTabelas();
	
	/* aloca estrutura a ser passada para a thread e atribui valor as variavies */
	if (  (args = (TpArgumento *)malloc(sizeof(TpArgumento)))==NULL  ) {
		puts("Erro de memoria!");
		exit(1);
	}
	
	// TODO ipEscuta deve ser recebido na linha de comando!!!
	if(((args->listenfd = u_open((u_port_t) DadosAplicacao.meuIp.porta, DadosAplicacao.meuIpString)) == -1)) {
		 puts("Nao conseguiu abrir porta de escuta");
		 exit(1);
	}
	
	if(DadosAplicacao.principal) {
		// Cria a thread que ficara ouvindo requisicoes.
		falha = pthread_create(&idThread, NULL, &ConectaSocketPrincipal, NULL);
		
		// Captura erro em caso de falha na criacao da thread.
		if(falha) {
			puts("Erro ao criar thread para ouvir requisicoes");
		}
		
		strcpy(args->ipConnect, "");
		args->portaConnect = -1;
	}
	else {
		strcpy(args->ipConnect, servidor);
		args->portaConnect = (u_port_t) DadosAplicacao.noPrincipalIp.porta;
	}
		
	SelecionaDescritores(args);
	
	exit(0);
}
