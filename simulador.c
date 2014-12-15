#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>

#define OP_INTERVAL     10      /* em segundos */
#define BUFFSIZE        100
#define DEFAULT_PORT    3000

////////////////////////////////////////////////////////////////////////////////

char    p2p_path[100]; /* caminho para o programa p2p */

int     hello_time, 
        response_time, 
        live_time;

typedef struct
{
    char id;
    char * ip;
    int port;
    int stdinNode[2];
    int pid;
} NODE;

NODE    *nodes;
int     n_nodes = 0;

typedef struct
{
  char  *ip;
  int   port; /* última porta usada para esse ip */
} PRINCIPAL_NODE;

PRINCIPAL_NODE  principal_node[3];

int     last_princ_node = 0;    /*  indicador de qual foi a última 
porta 
                                    usada no vetor de portas */


/////////////////////////////////////////////////////////////////////////////

/* 
 * Inicializa um nó
 * Se o id for A, B ou C é nó principal
 */
void initializeNode( char my_id )
{
    int i = 0, 
        j = 0, 
        pid = 0, 
        principal = 0;
        
    char    ** args, 
            temp[100];


    if( my_id != 'A' && my_id != 'B' && my_id != 'C' )
    {
        args = (char **)malloc( sizeof( char * ) * 11 );
        principal = 0;
    }
    else
    {
        args = (char **)malloc( sizeof( char * ) * 10 );
        principal = 1;
    }

   
    /* Atribuindo um ip */
    nodes = ( NODE * )realloc( nodes, sizeof( NODE ) * ( n_nodes + 1 ) );
    nodes[n_nodes].id = my_id;
    nodes[n_nodes].port = principal_node[last_princ_node].port;
    nodes[n_nodes].ip = strdup( principal_node[last_princ_node].ip );

    principal_node[last_princ_node].port += 3;
    last_princ_node = ( last_princ_node + 1 ) % 3;

#ifdef __RSH__
    args[i++] = strdup( "rsh" );
    
    args[i++] = strdup( nodes[n_nodes].ip );
#endif
    
    args[i++] = strdup( p2p_path );
    
    sprintf( temp, "%d", hello_time );
    args[i++] = strdup( temp );

    sprintf( temp, "%d", response_time );
    args[i++] = strdup( temp );

    sprintf( temp, "%d", live_time );
    args[i++] = strdup( temp );

    // novo nó
    sprintf( temp, "%s:%d", nodes[n_nodes].ip, nodes[n_nodes].port );
    args[i++] = strdup( temp );

    if( !principal )
    {
        sprintf( temp, "%s:%d", principal_node[0].ip, DEFAULT_PORT );
        args[i++] = strdup( temp );

        sprintf( temp, "%s:%d", principal_node[1].ip, DEFAULT_PORT+1 );
        args[i++] = strdup( temp );

        sprintf( temp, "%s:%d", principal_node[2].ip, DEFAULT_PORT+2 );
        args[i++] = strdup( temp );
    }
    else /* �? um nó principal, passar os vizinhos */
    {
        for( j = 0; j < 3; j++ )
        {
            // Não incluir vc mesmo
            if( n_nodes != j )
            {
                sprintf( temp, "%s:%d", principal_node[j].ip, DEFAULT_PORT + j );
                args[i++] = strdup( temp );
            }
        }
    }

    args[i++] = NULL; 
    
    fprintf( stdout, "[Simulador]: Iniciar nó %c\n", my_id );
    fprintf( stdout, "[Simulador]: $" );

    for( i = 0; args[i] != NULL; i++ )
    {
        fprintf( stdout, " %s", args[i] );
    }
    fprintf( stdout, "\n" );
    
    if( pipe( nodes[n_nodes].stdinNode ) < 0 )
    {
        perror( "[Simulador]: pipe" );
    }

    pid = fork();

    // child
    if( pid < 0 )
    {
        perror( "[Simulador]:fork" );
        exit( 1 );
    }
    else if( pid == 0 )
    {
        /* Redirecionando o stdin do processo */
        dup2( nodes[n_nodes].stdinNode[0], 0 );
    
        execvp( args[0], args );
        
        perror( "[Simulador]: execvp" );
        exit( 1 );
    }
    else
    {
        nodes[n_nodes].pid = pid;
        n_nodes++;
    }
}

int main( int argc, char** argv )
{

    register int    i = 0,
                    j = 0;
                             
    char    buff[100],  
            *id = NULL, 
            *op = NULL, 
            *cp = NULL, 
            delimiters[] = ",\n";
   
    FILE    * fp_op, 
            * fp_temp;

    /* Se não tiver argumento usa o diretório corrente */
    if( argc < 7 )
    {
        fprintf( stderr, "[Simulador]: Número inválido de argumentos\n" );
        fprintf( stderr, "[Simulador]: %s caminho_p2p arquivo_temporizadores arquivo_operacoes ip_no_principal1 ip_no_principal2 ip_no_principal3\n", argv[0] );
        return 1;
    }
    /* Caminho do programa p2p */
    strcpy( p2p_path, argv[1] );
    strcat( p2p_path, "/p2p" );

    /* Arquivos de entrada */
    /* Abre arquivo com os temporizadores */
    fp_temp = fopen( argv[2], "r" );
    if( fp_temp == NULL )
    {
        perror( "[Simulador]: fopen" );
        return 1;
    }

    /* Abre arquivo com as operações */
    fp_op = fopen( argv[3], "r" );
    if( fp_op == NULL )
    {
        perror( "[Simulador]: fopen" );
        return 1;
    }
   
    /* Lê arquivo com os temporizadores */
    fscanf( fp_temp, "%d\n", &hello_time );
    fscanf( fp_temp, "%d\n", &response_time );
    fscanf( fp_temp, "%d\n", &live_time );
    fclose( fp_temp );

    /* Inicializa os nós principais */
    principal_node[0].ip   = strdup( argv[4] );
    principal_node[0].port = DEFAULT_PORT;
    principal_node[1].ip = strdup( argv[5] );
    principal_node[1].port = DEFAULT_PORT+1;
    principal_node[2].ip = strdup( argv[6] );
    principal_node[2].port = DEFAULT_PORT+2;

    fprintf( stdout, "\n[Simulador]: Iniciando Nós principais A, B e C\n" );
    initializeNode( 'A' );
    initializeNode( 'B' );
    initializeNode( 'C' );
    
    fprintf( stdout, "\n[Simulador]: Arquivo de Entrada\n" );

    while( fgets( buff, 100, fp_op ) != NULL )
    {
       sleep( OP_INTERVAL );

       cp = strdup( buff );
       id = strtok( cp, delimiters );
       op = strtok( NULL, delimiters );

       /* Se for operação de entrada, inicializar o nó */
       if( strcmp( op, "entrada" ) == 0 )
       {
           initializeNode( id[0] );
       }
       else /* Repassa a operação para o nó */
       {
           for( i = 0; i < n_nodes; i++ )
           {
               if( nodes[i].id == id[0] )
                   break;
           }

           op[ strlen( op ) ]='\n';
           fprintf( stdout, "[Simulador]: Mensagem para o nó %s: %s", id, op );
           write( nodes[i].stdinNode[1], op, strlen( op ) );
       }

    }
    fclose( fp_op );

    
    /* Enviando comando de saída para os nós principais */
    for( i = 0; i < 3; i++ )
    {
       fprintf( stdout, "[Simulador]: Mensagem para o nó %c: saída\n", 'A' + i );
       write( nodes[i].stdinNode[1], "saída\n", strlen( "saída\n" ) );
    }
    fprintf( stdout, "[Simulador]: Fim de operações.\n\n" );    
    
    
    /* Aguardando os processos terminarem */
    fprintf( stdout ,"[Simulador]: Aguardando os nós terminarem...\n" );    
    for( i = 0; i < n_nodes; i++ )
    {
       pid_t pid;
       int status = 0;

       pid = waitpid( -1, &status, 0 );
       
       for( j = 0; j < n_nodes; j++ )
            if( nodes[j].pid == pid )
                break;
                
       if( WIFEXITED(status) )
            fprintf( stdout ,"[Simulador]: Processo %c terminou legal.\n", nodes[j].id );
       else if( WIFSIGNALED( status ) ) 
            fprintf( stdout ,"[Simulador]: Processo %c terminou com sinal %d.\n", 
                     nodes[j].id, WTERMSIG(status) );
    }   

    return 0;

}