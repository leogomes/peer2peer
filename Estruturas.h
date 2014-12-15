#ifndef _ESTRUTURAS_H_
#define _ESTRUTURAS_H_

//bool operator==(const StrRote *rr1, const StrRote *rr2);

/* ------------------------------ constantes ------------------------------ */


#define MAX_ROTEAMENTO 100

// Eventos de TIMER
#define  TIMER_JOINR    0
#define  TIMER_HELLO  	1
#define  TIMER_HELLOR	2
#define  TIMER_SEARCHR	3
#define  TIMER_GETR		4

#define QTD_TIPOS_MSGS      7
#define QTD_TIPOS_MSGS_RESP 3

// Tipos de Mensagem
#define JOIN 	1
#define JOINR 	2
#define HELLO 	3
#define SEARCH 	4
#define SEARCHR 5
#define GET 	6
#define GETR	7

#define MAX_CMD_ENTRADA 1024
#define MAX_VIZINHOS 6
#define MIN_VIZINHOS 2

#define MAX_NOME_ARQ 256

#define TAM_BLOCO 400	// Tamanho em bytes do bloco enviado de arquivo enviado

#define SOCKET_DESCONHECIDO -2
/* ------------------------------ estruturas ------------------------------ */

// Identificacao do no'.
typedef struct {
	char oc1, oc2, oc3, oc4; // oc1 mais significativo
	unsigned int porta;	
} IdPeer;

// Header das mensagens.
typedef struct {
	char tipoMsg;
	IdPeer remetente;
	IdPeer destino;
	short numSeq;
	int tamDados;
} Header;

/************ 
	Tabelas
 *************/

typedef struct {
	IdPeer peer;
	int sockfd;
} RegSocket;
 
typedef struct {
	IdPeer vizinho;
	int sockfd;
} RegVizinho;

typedef struct{
	IdPeer vizinho;
	int distancia;
	int sockfd;
} TempRegVizinho;

typedef struct{
	IdPeer peer;
	int distancia;
	char nomeArquivo[MAX_NOME_ARQ];
} RegSearch;

typedef struct{
	Header header;
	int sockfd;
	char nomeArquivo[MAX_NOME_ARQ];
} TArgumentoGet;

typedef struct{
	char nomeArquivo[MAX_NOME_ARQ];
	bool recebendo;
	int ignoraTimeouts;
} RegGetResponse;




// Armazenamento do ultimo numero de sequencia de um tipo de mensagem e se ainda e' valido.
typedef struct{
	char tipoMsg;
	short numSeq;
	bool valido;
} NumeroSequencia;


// Roteamento de mensagens.



typedef struct {
	char tipoMsg;
	IdPeer origem;
	short numSeq;
	IdPeer vizinho;
} RegRoteamento;

// acesso individual a cada byte do numero da porta.
typedef union {
	unsigned porta_int;
	unsigned char porta_byte[sizeof(unsigned)];
} tipo_porta;


// Prototipo.
void createHeader(Header *h, char tipoMsg, IdPeer destino);

void IdPeer2StrIp(char * str, IdPeer id);

IdPeer string2IdPeer(char *id);

unsigned char calcDistancia(IdPeer *peer1, IdPeer *peer2);

short numSequencia(unsigned char tipoMsg);

//VizinhoSocket* RetornaVizinhoPorIdPeer(const std::list<VizinhoSocket>& listaVizinho, const IdPeer& vizinho);

//VizinhoSocket* RetornaVizinhoPorSocket(const std::list<VizinhoSocket>& listaVizinho, const IdPeer& vizinho);

//bool RemoveVizinho(std::list<VizinhoSocket>& listaVizinho, const IdPeer& vizinho);

//bool InsereVizinho(std::list<VizinhoSocket>& listaVizinho, const IdPeer& vizinho);

void InicializaTabelaMensagensValidas();

int ConverteTipoMsgParaIndice(int tipo);

void InsereMensagemResposta(short numSeq, char tipoMsg);

void RemoveMensagemResposta(int tipoMsg);

bool MensagemEhValida(int , short);

bool RetornaRegRoteamento(IdPeer& vizinho, const IdPeer& origem, char tipoMsg, short numSequencia);

bool InsereTabRoteamento(const IdPeer& vizinho, const IdPeer& origem, char tipoMsg, short numSequencia);

void imprimeRoteamento();

bool isReceived( Header& h );

bool isValida(int tipoMsg);

char *toLower(char *str);

int buscaSocketVizinho(const IdPeer&);

int buscaPosicaoVizinho(const IdPeer& vizinho);

bool IdPeerIguais(const IdPeer& id1, const IdPeer& id2);

void defineVizinhos();

int contaVizinhos();


bool existeTabelaSocket(int sock);
bool insereTabelaSocket(int sock);
bool removeTabelaSocket(int sock);
int encontraTabelaSocketPorRSet(fd_set rset);

void setNomeArquivoSearch(char *);
void inicializaSearch();
void incrementaPacotesRecebidos();
void decrementaPacotesRecebidos();
int retornaPacotesRecebidos();
void escolheSearch(IdPeer &, int );
void defineSearch();

void inicializaGetResponse(char *arquivo);
void getNomeArquivo(char *nome);
bool getRecebendo();
void setRecebendo(bool pEstado);


void printHelp();

void imprimeTabelaVizinhos();

void distribui( Header& , char *);
void distribuiGet(Header& h, char * dadosP);
void encaminha(int peerfd, Header& h, char * dadosP);


// Adicionados
void imprimeTabelaVizinhosTemp();
void InicializaTabelaTempVizinhos();
void InicializaTabelaVizinhos();
void escolheVizinhos(const IdPeer& vizinho, int distancia, int sockfd);
bool insereVizinho(const IdPeer& vizinho, int sockfd);
bool removeVizinho(const IdPeer& vizinho);
void InicializaTabelas();
char converteTipoMsg(char tipo);
void verificaTabelaVizinhosSocketsDesconhecidos();
bool atualizaIdPeerTabelaSocket(int sockP, IdPeer vizinhoP);
bool ehMensagemRoteada(char tipoMsg);
bool buscaIdPeerVizinho(int sockfd, IdPeer &vizinho);
int encontraTabelaSocketPorIdPeer(IdPeer& vizinhoP);
bool existeTabelaSocket(int sock);
bool insereTabelaSocket(int sock);
bool atualizaTabelaSocketPorIdPeer(IdPeer& vizinhoP, int pSockfd);
void imprimeTabelaSocket();

#endif
