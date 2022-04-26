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


/******************************************************************************
* PrintStatment()
*
* Arguments: num - seleciona qual a informação a mostrar no stdout
*
* Returns: void
*
* Descrição: Consoante a situação imprime os comandos válidos na interface de
*            utilizador ou explicita o formato correto para invocar a aplicação
*****************************************************************************/
void PrintStatment(int num){
  if(num == 2){
    fprintf(stdout, "\n************************************************************** \n");
    fprintf(stdout, "\nArgumentos de entrada tem forma:");
    fprintf(stdout, "\nring i i.IP i.Port");
    fprintf(stdout, "\nIP: Endereco IP da maquina que aloja a aplicacao");
    fprintf(stdout, "\nPort: Porto da aplicacao, ou seja, o porto tanto de um processo servidor TCP como de um processo servidor UDP a ser instalado na maquina para concretizacao da aplicacao\n");
    fprintf(stdout, "\n************************************************************** \n");
  } else if(num==3){
    fprintf(stdout, "\n************************************************************** \n");
    fprintf(stdout, "\nLista de comandos:");
    fprintf(stdout, "\n->new:\n  n - Criacao de um novo anel composto exclusivamento pelo nó i.");
    fprintf(stdout, "\n->bentry:\n  b boot boot.IP boot.port - Entrada do nó no anel ao qual pertence o nó boot com endereço IP boot.IP e porto boot.port");
    fprintf(stdout, "\n->pentry:\n  p pred pred.IP pred.port - Entrada do nó no anel sabendo que o seu predecessor será o nó pred com endereço IP pred.IP e porto pred.port");
    fprintf(stdout, "\n->leave:\n  l - Saida do nó do anel.");
    fprintf(stdout, "\n->show:\n  s - Mostra o estado do nó, incluindo a sua chave, endereco IP e porto, bem como os valores correspondentes do seu predecessor e sucessor.");
    fprintf(stdout, "\n->chord:\n  c i i.IP i.port - Criação de um atalho para o nó i com endereço IP i.IP e porto i.port.");
    fprintf(stdout, "\n->dchord:\n  d - Eliminação do atalho");
    fprintf(stdout, "\n->find:\n  f k - Pesquisa do nó que armazena a chave k, com a apresentacao da sua chave, endereco IP e porto.");
    fprintf(stdout, "\n->exit:\n  e - Termina a aplicacao.\n");
    fprintf(stdout, "\n************************************************************** \n");

  }
}

/******************************************************************************
* DistanceFind()
*
* Arguments: node - estrutura do tipo Node que contém informação sobre um nó;
*            k ---- inteiro que contém a chave que pretende procurar.
*
* Returns: int - distância entre dois nós.
*
* Descrição: Calcula a distância entre dois nós.
*****************************************************************************/
int DistanceFind(Node *node ,int k ){
  int result_curr = 0;
  int result_succ = 0;
  int result_chord = 0;
  result_curr = k - node->curr->key;
  result_succ = k - node->succ->key;
  if (node->chord->key != -1) {
    result_chord = k - node->chord->key;
    if (result_chord < 0) {
      result_chord = result_chord +32;
    }
  }
  if (result_curr < 0) {
    result_curr = result_curr + 32;
  }
  if (result_succ < 0) {
    result_succ = result_succ + 32;
  }

  if (node->chord->key != -1) {
    if ((result_curr <= result_succ) && (result_curr <= result_chord)) {
      return node->curr->key;
    } else if((result_succ <= result_curr) && (result_succ <= result_chord)){
      return node->succ->key;
    } else {
      return node->chord->key;
    }
  } else{
    if ((result_curr <= result_succ)) {
      return node->curr->key;
    } else{
      return node->succ->key;
    }
  }
}

/******************************************************************************
* Init_node()
*
* Arguments: node - estrutura do tipo Node que contém informação sobre um nó.
*
* Retorna: Node* - estrutura do tipo Node que contém informação sobre um nó.
*
* Descrição: Aloca a memória necessária e inicializa os campos que contêm
*            informação sobre um nó.
*****************************************************************************/
Node* Init_node(Node *node){
  node->pred = (Node_1*)malloc((sizeof(struct Node_1)));
  node->curr = (Node_1*)malloc((sizeof(struct Node_1)));
  node->succ = (Node_1*)malloc((sizeof(struct Node_1)));
  node->chord = (Node_1*)malloc((sizeof(struct Node_1)));
  node->curr->key = -1;
  node->pred->key = -1;
  node->succ->key = -1;
  node->chord->key = -1;
  return node;
}

/******************************************************************************
* FreeNode()
*
* Arguments: node - estrutura do tipo Node que contém informação sobre um nó.
*
* Retorna: void
*
* Descrição: Liberta a memória alocada para guardar a informação de um nó.
*****************************************************************************/
void FreeNode(Node *node){
  free(node->pred);
  free(node->curr);
  free(node->succ);
  free(node->chord);
}
