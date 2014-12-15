/*  UFRJ - DCC - Teleprocessamento e Redes - 2004/2
	Fabio Pereira Sarmento - DRE:
	Gustavo Zopelari Nasu - DRE: 098212015
	Leonardo Freitas Gomes - DRE: 098211336
	Ricardo Chiganer Lilenbaum - DRE: 101133019 */
	
#include "mysock.h"
#include <malloc.h>

/* ------------------------------ u_open ------------------------------ */

int u_open(u_port_t porta, char *ipEscuta)
{
	int sockfd;
	struct sockaddr_in server;
	struct in_addr enderecoIp;

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) return -1;

	if( ( inet_aton(ipEscuta, &enderecoIp) )  == 0 ) {
		close(sockfd);
		return -1;
	}

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	//server.sin_addr.s_addr = htonl(enderecoIp.s_addr);
	server.sin_port = htons(porta);

	if( (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) <0) ||
		 (listen(sockfd, MAXBACKLOG) < 0)) {
		close(sockfd);
		return -1;
	}

	return sockfd;
}

/* ------------------------------ u_close ------------------------------ */

int u_close(int sockfd)
{
	return close(sockfd);
}

/* ------------------------------ u_accept ------------------------------ */

int u_accept(int sockfd, char *hostn)
{
	struct sockaddr_in net_client;
	socklen_t len = (socklen_t)sizeof(struct sockaddr);
	int retval;
	struct hostent *hostptr;

	while( ((retval= accept(sockfd, (struct sockaddr *)(&net_client), &len)) == -1) && (errno == EINTR));

	if(retval == -1) return retval;

	hostptr = gethostbyaddr((char *)&(net_client.sin_addr.s_addr), 4, AF_INET);

	if(hostptr == NULL) strcpy(hostn, "unknown");
	else strcpy(hostn, hostptr->h_name);

	return retval;
}

/* ------------------------------ u_connect ------------------------------ */

int u_connect(u_port_t porta, char *hostn)
{
	struct sockaddr_in server;
	struct hostent *hp;
	int sockfd;
	int retval;

	hp = gethostbyname(hostn);
	if( !(hp = gethostbyname(hostn)) || ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) )
		return -1;

	memcpy((char *)&server.sin_addr, hp->h_addr_list[0], hp->h_length);

	server.sin_port = htons(porta);
	server.sin_family = AF_INET;

	while( ((retval= connect(sockfd, (struct sockaddr *)&server, sizeof(server))) == -1) && (errno == EINTR));
	if(retval == -1) {
		close(sockfd);
		return retval;
	}

	return sockfd;
}

/* ------------------------------ u_recv ------------------------------ */

ssize_t u_recv(int fd, void *buf, size_t size)
{
	ssize_t retval;
	size_t ja_veio, ainda_vem;

	for(ja_veio=0, ainda_vem=size; ja_veio < size; ja_veio+= retval, ainda_vem-= retval)
	{
		while(retval = read(fd, (char *)buf+ja_veio, ainda_vem), retval == -1 && errno == EINTR);
		if(retval== -1) break;
	}

	return retval;
}

/* ------------------------------ u_send ------------------------------ */

ssize_t u_send(int fd, void *buf, size_t size)
{
	ssize_t retval;
	size_t ja_foi, ainda_vai;

	for(ja_foi=0, ainda_vai=size; ja_foi < size; ja_foi+= retval, ainda_vai-= retval)
	{
		while(retval = write(fd, (char *)buf+ja_foi, ainda_vai), retval == -1 && errno == EINTR);
		if(retval== -1) break;
	}

	return retval;
}

/* ------------------------------ lePacote ------------------------------ */
int lePacote(int fd, Header& header, char * buffP)
{
	ssize_t retval;
	char * buffer = NULL;
	
//	printf("\nLENDO PACOTE\n");
	
	// Le o 'Header' do pacote.
	retval = u_recv(fd, &header, sizeof(Header));
//	printf("TIPO PACOTE: %d\n", header.tipoMsg);
	
	if(retval == -1)
		fprintf(stderr, "Erro na leitura do Header do Pacote.");
	
	if(header.tamDados != 0) {
		buffer = (char *)malloc(header.tamDados);
		
	//	printf("LENDO DADOS\n");
		
		// Le os dados do pacote.
		retval = u_recv(fd, buffer, header.tamDados);
		
		if(retval == -1)
			fprintf(stderr, "Erro na leitura dos dados do Pacote.");
		
		if( buffP != NULL)
			memcpy(buffP, buffer, retval);
	}
	else {
		if(buffP != NULL)
			strcpy(buffP, "");
	}
	
	if(buffer != NULL)
		free(buffer);


	
	return header.tamDados;
}


/* ------------------------------ escrevePacote ------------------------------ */

bool escrevePacote(int fd, Header& header, char * buffP)
{
	ssize_t retval;
	
	// Escreve o 'Header' do pacote.
	retval = u_send(fd, &header, sizeof(Header));
	
	if(retval == -1){
		fprintf(stderr, "Erro na escrita do header do pacote.\n");
		return false;
	}
	
	if((buffP != NULL) || (header.tamDados == 0)) {
		// Escreve os dados do pacote.
		retval = u_send(fd, buffP, header.tamDados);
		if(retval == -1){
			fprintf(stderr, "Erro na escrita dos dados do pacote.\n");
			return false;
		}
	}
	return true;
}
