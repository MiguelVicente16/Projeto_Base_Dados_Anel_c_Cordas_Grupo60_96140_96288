#include "Command.h"
#include "aux.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <time.h>
#define max(A,B) ((A)>=(B)?(A):(B));
#define MAX 1000

typedef struct Node_1{
  int key;
  int key_find;
  char ip[50];
  char port[50];
}Node_1;


typedef struct Node{
  struct Node_1 *curr;
  struct Node_1 *pred;
  struct Node_1 *succ;
  struct Node_1 *chord;
}Node;

/******************************************************************************
* succdc()
*
* Arguments: node - estrutura do tipo Node que contém informação sobre um nó;
*            KEY -- chave de um nó;
*            IP --- endereço IP do servidor;
*            PORT - porto utilizado para tráfego de informação no servidor.
*
* Retorna: Node* - estrutura do tipo Node que contém informação sobre um nó;
*
* Descrição: Estabelece uma conexão TCP e atualiza os dados do predecessor.
*****************************************************************************/
Node *succdc(Node *node, int KEY, char * IP, char * PORT){

  char message[MAX], returnmessage [MAX];
  int n;
  tcpPredSocket=-1;

  //atualiza estrutura passando o  para sucessor
  node->pred->key=KEY;
  strcpy(node->pred->ip, IP);
  strcpy(node->pred->port, PORT);
  if(node->curr->key!=node->pred->key){
    //cria ligacao TCP com novo predecessor e o avisa
    sprintf(returnmessage,"SELF %d %s %s\n", node->curr->key, node->curr->ip, node->curr->port);
    tcpSuccSocket=TCPWriteMessage(node->pred->ip, node->pred->port, returnmessage);
    if(tcpSuccSocket==-1){
      printf("Nao foi possivel realizar a conexao TCP\n");
      return node;
    }
  }
  //caso tenha ficado sozinho no anel
  else tcpPredSocket=-1;
  return node;
}

/******************************************************************************
* TCPWrite()
*
* Arguments: fd -- file descriptor com o destino da mensagem;
*            str - ponteiro para string que contém a mensagem que se pretende
*                  enviar.
*
* Retorna: int - informa sobre a existência de erro;
*
* Descrição: Envia mensagem através de conexão TCP.
*****************************************************************************/
int TCPWrite(int fd, char* str){
  int charleft=strlen(str);
  if (write(fd, str, charleft) == -1) {
    printf("err connect1\n"); exit(1);
  }
  return 0;
}

/******************************************************************************
* TCPWriteMessage()
*
* Arguments: message - ponteiro para string que contém a mensagem que se
*                      pretende enviar;
*            PORT ---- porto de servidor que se vai abrir;
*            IP ------ endereço IP do servidor que se vai abrir.
*
* Retorna: int - socket para onde é enviada a mensagem ou então informação de
*                fecho da socket.
*
* Descrição: Abre um cliente TCP e invoca TCPwrite para enviar a mensagem.
*
*****************************************************************************/
int TCPWriteMessage(char IP[MAX], char PORT[MAX], char *message){
  int errcode;
  ssize_t n = 0;
  struct addrinfo hints, *res;
  int tcpSocket = 0;

  struct sigaction act;
  memset(&act,0,sizeof act);
  act.sa_handler=SIG_IGN;
  if(sigaction(SIGPIPE,&act,NULL)==-1)/*error*/exit(1);

  tcpSocket=socket(AF_INET,SOCK_STREAM,0);
  if(tcpSocket==-1) {printf("err socket\n"); close(tcpSocket); return -1;}
  memset(&hints,0,sizeof hints);
  hints.ai_family=AF_INET;
  hints.ai_socktype=SOCK_STREAM;
  hints.ai_flags=AI_CANONNAME;
  errcode=getaddrinfo(IP,PORT, &hints,&res);
  if(errcode!=0) {printf("err addrinfo\n"); close(tcpSocket); return -1;}
  n = connect(tcpSocket, res->ai_addr,res->ai_addrlen);
  if (n==-1) {printf("err connect2\n"); close(tcpSocket); return -1;}
  n = TCPWrite(tcpSocket, message);
  if(n==-1) { close(tcpSocket); return -1;}

  maxfd = max(maxfd, tcpSocket);
  freeaddrinfo(res);
  return tcpSocket;
}

/******************************************************************************
* UDPWriteMessage()
*
* Arguments: message - ponteiro para string que contém a mensagem que se
*                      pretende enviar;
*            PORT ---- porto do servidor que se vai abrir;
*            IP ------ endereço IP do servidor que se vai abrir.
*
* Retorna: int - file descriptor.
*
* Descrição: Abre um cliente UDP, envia uma mensagem e recebe outra mensagem que
*            confirma a receção da anterior (ACK).
*****************************************************************************/
int UDPWriteMessage(char *IP, char *PORT, char *message){

  struct addrinfo hints,*res;
  int fd,errcode;
  ssize_t n;
  struct sockaddr addr;
  socklen_t addrlen;
  char buffer[MAX];
  fd = socket(AF_INET,SOCK_DGRAM,0);//UDP socket
  if(fd==-1)/*error*/exit(1);
  memset(&hints,0,sizeof hints);
  hints.ai_family=AF_INET;//IPv4
  hints.ai_socktype=SOCK_DGRAM;//UDP socket

  errcode=getaddrinfo(IP,PORT,&hints,&res);
  if(errcode!=0)/*error*/exit(1);

  n=sendto(fd,message,strlen(message),0,res->ai_addr,res->ai_addrlen);
  if(n==-1)/*error*/exit(1);
  n=recvfrom(fd,buffer,MAX,0, (struct sockaddr*)&addr,&addrlen);
  if(n==-1) /*error*/exit(1);
  freeaddrinfo(res);
  return fd;
}

/******************************************************************************
* bentry()
*
* Arguments: node - estrutura do tipo Node que contém informação sobre um nó;
*            k ---- chave por quem vai procurar;
*            KEY -- chave do nó para onde se vai mandar mensagem;
*            IP --- endereço IP do nó para onde se vai mandar a mensagem;
*            PORT - porto do nó para onde se vai mandar a mensagem.
*
* Retorna: Node* -  estrutura do tipo Node que contém informação sobre um nó;
*
* Descrição: Concretiza o comando "b" ou "bentry" em que se insere um nó no anel
*            a partir de um qualquer nó já presente no anel. Inicia o processo
*            de procura enviando a mensagem "EFND".
*****************************************************************************/
Node *bentry(Node *node,int k, int KEY, char *IP, char *PORT){
  char message[MAX];
  sprintf(message, "EFND %d", k);
  udpsocket = UDPWriteMessage(IP,PORT,message);
  return node;
}

/******************************************************************************
* chord()
*
* Arguments: node ------- estrutura do tipo Node que contém informação sobre um
*                         nó;
*            key_chord -- chave de nó de destino da corda;
*            PORT_chord - porto do nó de destino da corda.
*
* Retorna: Node* - estrutura do tipo Node que contém informação sobre um nó.
*
* Descrição: Concrertiza o comando "c" ou "chord" em que se cria uma corda
*            (atalho) para um servidor do anel através de uma sessão UDP.
*****************************************************************************/
Node *chord(Node *node, int key_chord, char *IP_chord, char *PORT_chord){

  if (node->chord->key != -1) {
    printf("****************************************** \n\n");
    printf("\tab\nJá tem um atalho. Deve fazer dchord primeiro - d\n");
    printf("****************************************** \n\n");
    return node;
  }
  node->chord->key = key_chord;
  strcpy(node->chord->ip, IP_chord);
  strcpy(node->chord->port, PORT_chord);

  return node;
}

/******************************************************************************
* find()
*
* Arguments: node ----- estrutura do tipo Node que contém informação sobre um
*                       nó;
*            key_find - chave que se está a procurar.
*
* Retorna: Node* - estrutura do tipo Node que contém informação sobre um nó.
*
* Descrição: Concrertiza o comando "f" ou "find" em que se procura uma chave no
*            anel calculando as distâncias entre nós e a chave que pretendemos
*            encontrar. Caso encontre a chave envia a resposta("RSP") caso
*            contrario continua a procura("FND").
*****************************************************************************/
Node *find(Node *node, int key_find){
  int num = 0;
  srand(time(NULL));
  int n = rand()%99, len;
  int key_result = DistanceFind(node,key_find);
  char message[MAX];
  if (key_result == node->curr->key) {
    node->curr->key_find = key_find;
    if (node->succ->key == node->curr->key) {
      efnd = 0;
    } else{
      sprintf(message, "RSP %d %d %d %s %s\n",  node->curr->key, n, node->curr->key, node->curr->ip, node->curr->port);
      num = TCPWrite(tcpSuccSocket, message);
      if (num==-1) {close(tcpSuccSocket); exit(1);}
    }
    //Para a procura e inicia a resposta, foi encontrado o objeto neste nó
  } else if (key_result == node->succ->key) {
    node->curr->key_find = key_find;
    sprintf(message, "FND %d %d %d %s %s\n",  node->curr->key_find, n, node->curr->key, node->curr->ip, node->curr->port);
    num = TCPWrite(tcpSuccSocket, message);
    if (num==-1) {close(tcpSuccSocket); exit(1);}
    //Continua a procura no proximo nó
  } else if(key_result == node->chord->key){
    node->curr->key_find = key_find;
    sprintf(message, "FND %d %d %d %s %s",  node->curr->key_find, n, node->curr->key, node->curr->ip, node->curr->port);
    chordsocket = UDPWriteMessage(node->chord->ip, node->chord->port,message);
    if (chordsocket == -1) {close(chordsocket); exit(1);}
    //Continua a procura no atalho
  }
  return node;
}

/******************************************************************************
* leave()
*
* Arguments: node - estrutura do tipo Node que contém informação sobre um nó.
*
* Retorna: Node* - estrutura do tipo Node que contém informação sobre um nó.
*
* Descrição: Concretiza o comando "l" ou "leave" em que se remove um nó do anel.
*            Envia uma mensagem("PRED") para o seu sucessor, fecha a socket do
*            sucessor e atualiza a estrutura node.
*****************************************************************************/
Node *leave(Node *node){

  sair = 1;
  char message[MAX];
  int n = 0;
  sprintf(message, "PRED %d %s %s\n", node->pred->key, node->pred->ip, node->pred->port);
  n = strlen(message);
  if (write(tcpSuccSocket, message, n) == -1) {
    printf("err connect3\n"); exit(1);
  }
  close(tcpSuccSocket); tcpSuccSocket=-1;
  //atualiza a estrutura
  node->succ->key = -1;
  node->pred->key = -1;
  flag = 2;
  return node;
}

/******************************************************************************
* show()
*
* Arguments: node - estrutura do tipo Node que contém informação sobre um nó.
*
* Retorna: void
*
* Descrição: Concretiza o comando "s" ou "show" em que se mostra a informação do
*            servidor em questão(informação do nó corrente, do predecessor,
*            sucessor e da corda - caso exista).
*****************************************************************************/
void show(Node* node){
  if (node->pred->key == -1 && node->succ->key==-1) {
    printf("\n******* Informacao de estado no nó ******* \n\n");
    printf("      O nó não pertence a nenhum anel\n\n");
    printf("****************************************** \n\n");
  } else {
    printf("******* Informacao de estado no nó %d ******* \n\n", node->curr->key);
    printf("\tPred key = |%d|\n\tPred IP = |%s|\n\tPred Port = |%s|\n\n", node->pred->key, node->pred->ip, node->pred->port);
    printf("\tCurr key = |%d|\n\tCurr IP = |%s|\n\tCurr Port =|%s|\n\n", node->curr->key, node->curr->ip, node->curr->port);
    printf("\tSucc key = |%d|\n\tSucc IP = |%s|\n\tSucc Port = |%s|\n\n", node->succ->key, node->succ->ip, node->succ->port);
    if (node->chord->key != -1) {
      printf("\tChord key = |%d|\n\tChord IP = |%s|\n\tChord Port = |%s|\n\n", node->chord->key, node->chord->ip, node->chord->port);
    }
    printf("********************************************* \n\n");
  }
}

/******************************************************************************
* pentry()
*
* Arguments: node ------ estrutura do tipo Node que contém informação sobre um
*                        nó;
*            key_pred -- chave do nó predecessor;
*            IP_pred --- endereço IP do servidor do nó predessor;
*            PORT_pred - porto do servidor do nó predessor.
*
* Retorna: Node* - estrutura do tipo Node que contém informação sobre um nó.
*
* Descrição: Concretiza o comando "p" ou "pentry" em que insere um nó num anel
*            preexistente em que é conhecida a informação do seu predecessor.
*            Envia uma mensagem("SELF") para o predecessor e atualiza a
*            estrutura com os dados do predecessor.
*****************************************************************************/
Node *pentry(Node *node, int key_pred, char IP_pred[], char PORT_pred[]){

  if ((key_pred != node->curr->key)) {
    char message[MAX];
    //para o caso de ja ter havido algum attempt falho
    if(tcpSuccSocket!=-1) close(tcpSuccSocket);
    //abre sessao TCP enviando mensagem NEW
    sprintf(message, "SELF %d %s %s\n", node->curr->key, node->curr->ip, node->curr->port);
    if (strlen(PORT_pred)>5) {
      PORT_pred[5] = '\0';
    }
    tcpPredSocket=TCPWriteMessage(IP_pred,PORT_pred,message);
    if(tcpPredSocket==-1){printf("Erro ao conectar com o futuro predecessor\n"); return node;}
    //atualiza a estrutura
    node->pred->key = key_pred;
    strcpy(node->pred->ip, IP_pred);
    strcpy(node->pred->port, PORT_pred);
  } else{
    printf("**************************************************** \n\n");
    printf("    O nó %d a inserir já pertence ao anel\n", node->curr->key);
    printf(" Faça |e| ou |exit| e volte a invocar o programa\n");
    printf("**************************************************** \n\n");
  }
  return node;

}

/******************************************************************************
* new()
*
* Arguments: node - estrutura do tipo Node que contém informação sobre um nó;
*            KEY -- chave do nó criador do anel;
*            IP --- endereço IP do né criador do anel;
*            PORT - porto do nó criador do anel.
*
* Retorna: Node* - estrutura do tipo Node que contém informação sobre um nó.
*
* Descrição: Concretiza o comando "n" ou "new" em que se abre um anel com o
*            primeiro nó.
*****************************************************************************/
Node *new(Node* node, int KEY, char *IP, char *PORT){
  node->curr->key = KEY;
  strcpy(node->curr->ip, IP);
  strcpy(node->curr->port, PORT);
  node->pred->key = KEY;
  strcpy(node->pred->ip, IP);
  strcpy(node->pred->port, PORT);
  node->succ->key = KEY;
  strcpy(node->succ->ip, IP);
  strcpy(node->succ->port, PORT);
  return node;
}
