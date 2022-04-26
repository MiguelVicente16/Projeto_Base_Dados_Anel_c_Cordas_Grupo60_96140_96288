#include "Command.h"
#include "Interface.h"
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
#define MAX 500

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
* interface()
*

* Arguments: node - estrutura do tipo Node que contém informação sobre um nó;
*            KEY -- chave de um nó;
*            IP --- endereço IP do servidor;
*            PORT - porto utilizado para tráfego de informação no servidor.
*
* Retorna: Node* - estrutura do tipo Node que contém informação sobre um nó.
*
* Descrição: Lida com imput proveniente da interface de utilizador e verfica
*            qual a operação que se pretende realizar.
*****************************************************************************/
Node * interface(Node *node, int KEY, char IP[MAX], char PORT[MAX]){

  char buffer[MAX], command[MAX], ip_new[MAX], port_new[MAX], message[MAX];
  int n, key_new, ip = 0, port = 0;
  char aux_IP[MAX] = "\0" ;
  char aux_PORT[MAX] = "\0";


  if (fgets(buffer, 1024, stdin) == NULL) {
    fprintf(stderr, "ERROR: %s\n", strerror(errno));
    exit(-1);
  }
  fprintf(stdout,"\n");
  print = 1;
  n = sscanf(buffer, "%s %d %s %s\n", command, &key_new, ip_new, port_new);
  if (n == 4) {
    strcpy(aux_PORT,port_new);
    strcpy(aux_IP,ip_new);
  }

  if(n == 0){printf("Nenhum comando recebido\n"); return node;}
  if ((strcmp(command, "n") == 0 || strcmp(command, "new") == 0) && (n == 1)) {
    newring = 1;
    printf("********************************************* \n\n");
    printf("  Criacao de um novo anel com chave %d\n\n", KEY );
    printf("********************************************* \n\n");
    node = new(node,KEY,IP,PORT);
    flag = 0;
    print = 0;
    sair = 0;
    return node;
  } else if ((strcmp(command, "p") == 0 || strcmp(command, "pentry") == 0) && (n == 4)){
    sair = 0;
    if((node->pred->key!= -1) && (node->succ->key != -1)){
      printf("Ja pertence a um anel\n");
    }
    else if (KEY != key_new){
      printf("\nO nó %d é predecessor de ip %s e porto %s\n", key_new, aux_IP, aux_PORT);
      node = new(node,KEY,IP,PORT);
      node = pentry(node, key_new, aux_IP, aux_PORT);
    } else{
      printf("**************************************************** \n\n");
      printf("    O nó %d a inserir já pertence ao anel\n", KEY);
      printf(" Faça |e| ou |exit| e volte a invocar o programa\n");
      printf("**************************************************** \n\n");
    }
    flag = 0;
    print = 0;
    return node;
  } else if ((strcmp(command, "s") == 0 || strcmp(command, "show") == 0) && (n == 1)){
    show(node);
    flag = 0;
    return node;
  } else if ((strcmp(command, "l") == 0 || strcmp(command, "leave") == 0) && (n == 1)){
    if ((node->curr->key == node->pred->key) && (node->curr->key == node->succ->key)) {
      bentry_t = 0;
      close(tcpSuccSocket); tcpSuccSocket=-1;
      node->succ->key = -1;
      node->pred->key = -1;
      flag = 2;
      return node;
    } else{
      bentry_t = 0;
      node = leave(node);
      flag = 2;
      return node;
    }
  } else if ((strcmp(command, "f") == 0 || strcmp(command, "find") == 0) && (n == 2)){
    bentry_t = 0;
    node = find(node,key_new);
    return node;
  } else if ((strcmp(command, "c") == 0 || strcmp(command, "chord") == 0) && (n == 4)){
    if (node->chord->key == -1) {
      bentry_t = 0;
      node = chord(node,key_new, aux_IP, aux_PORT);
      return node;
    } else{
      printf("--------------------------------------------\n");
      printf("\tO nó %d já tem uma corda para o nó %d\n", node->curr->key, node->chord->key);
      printf("\tFaça primeiro | dchord | e depois volte a adicionar a nova corda\n");
      printf("--------------------------------------------\n");
      return node;
    }
  } else if ((strcmp(command, "e") == 0 || strcmp(command, "exit") == 0) && (n == 1)){
    if (((node->succ->key == -1) && (node->pred->key == -1))) {
      flag = 1;
      return node;
    } else if (((node->succ->key == node->curr->key) && (node->pred->key == node->curr->key))) {
      flag = 1;
      return node;
    } else return node;
  } else if ((strcmp(command, "d") == 0 || strcmp(command, "dchord") == 0) && (n == 1)){
    if (node->chord->key!= -1) {
      close(udpsocket);
      udpsocket = -1;
      close(chordsocket);
      chordsocket = -1;
      node->chord->key = -1;
    } else{ printf("O nó não tem cordas para eliminar\n");}
    return node;
  } else if ((strcmp(command, "b") == 0 || strcmp(command, "bentry") == 0) && (n == 4)){
    sair = 0;
    bentry_t = 1;
    flag = 0;
    node->curr->key = KEY;
    strcpy(node->curr->ip, IP);
    strcpy(node->curr->port, PORT);
    node = bentry(node,KEY, key_new, aux_IP, aux_PORT);
    return node;
  } else if((strcmp(command, "h") == 0 || strcmp(command, "help") == 0) && (n == 1)){
    PrintStatment(3);
    flag =  0;
    return node;
  } else {
    fprintf(stdout, "\n************************************************************** \n");
    printf("\n\tO comando inserido não é válido.\n" );
    printf("\tFaça |h| ou |help| para obter a lista de comandos\n");
    fprintf(stdout, "\n************************************************************** \n");
    flag = 0;
    return node;
  }
}
