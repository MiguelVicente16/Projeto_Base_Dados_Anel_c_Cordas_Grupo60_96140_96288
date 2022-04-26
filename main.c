/******************************************************************************
 IST           Ano letivo 2021/2022            2ºSemestre              3ºPeríodo

 Base-de-Dados em Anel com Cordas:
 Aplicação Ring - Projeto realizado no âmbito da Unidade Curricular de Redes de
 Computadores e Internet.

 Objetivo:
 Criar um anel com nós que comunicam entre si através de conexões TCP e/ou UDP.
 A aplicaçao desenvilvida é composta pelos elementos seguintes:
               1. comando de invocação da aplicação;
               2. interface de utilizador;
               3. protocolo de pesquisa de uma chave;
               4. protocolo para entrada e saída de um nó;
               5. protocolo para a descoberta da posição de um nó no anel.

 Ficheiros:
 1. main.c ------ contém a função main (programa principal);
 2. Servidor.c -- contém o código necessário para lidar com o  tráfego de
                  informação (abertura/fecho/envio/receção) entre servidores;
 3. Interface.c - contém o código que processa os imputs provenientes da
                  interface de utilizador;
 4. Command.c --- contém o código responsável por implementar as funcionalidades
                  dos diversos comandos provenientes da interface de utilizador;
 5. aux.c ------- contém o código com funções auxiliares ao funcionamento do
                  programa.

 Trabalho realizado por:                                        Data de entrega:
 Afonso Pereira - 96140                                         15/04/2022
 Miguel Vicente - 96288
 *****************************************************************************/

#include "Servidor.h"
#include "Interface.h"
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

int tcpSuccSocket;
int tcpPredSocket;
int tcpSocket;
int udpsocket;
int saveudpSocket;
int chordsocket;
char sender[MAX];
int efnd;
socklen_t addrlenudp;
struct sockaddr_in addrudp;
int flag;
int print;
int maxfd;
int bentry_t;
int Alone;
int sair;
int newring;
int count;
int efnd;
/******************************************************************************
* main()
*
* Arguments: argc - numero (inteiro) de argumentos passados quando o programa é
*                   invocado;
*            argv - ponteiro para vetor de strings que contém os argumentos
*                   passados.
*
* Returns: int status
*
*Description: Programa principal
*****************************************************************************/
int main(int argc, char  *argv[]) {

  struct in_addr addr;
  char *initial_addr;
  tcpSuccSocket = -1;
  tcpPredSocket = -1;
  udpsocket = -1;
  Node node;
  if (argc == 4) {
    PrintStatment(3);
    if (inet_aton(argv[1], &addr)== 1) {
      OpenServer(argv, &node);
    }
  } else if(argc > 4){
    printf("Erro: demasiados argumentos.\n");
    PrintStatment(2);
  } else {
    printf("Erro: inserir porto e ip.\n");
    PrintStatment(2);
  }
  return 0;
}
