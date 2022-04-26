#include "Command.h"
#include "Servidor.h"
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
 * SendRSP()
 *
 * Arguments: n - número de sequência da resposta
              node - estrutura com informação do nó
              KEY - chave do nó para onde é destinada a mensagem
 * Retorna: num - 0 caso não dê erro a enviar a mensagem e -1 caso dê erro
 *
 * Descrição: Começa a enviar a resposta depois de encontrar o objeto.

 *
 *****************************************************************************/
int SendRSP(int n, Node* node, int KEY){
  int num = 0;
  char message[MAX];
  int key_result = DistanceFind(node,KEY);
  sprintf(message, "RSP %d %d %d %s %s\n",  KEY, n, node->curr->key,  node->curr->ip, node->curr->port);
  if (key_result == node->succ->key) {
    num = TCPWrite(tcpSuccSocket, message);
    if (num==-1) {close(tcpSuccSocket); exit(1);}
    //Continua a procura no proximo nó
  } else if (key_result == node->chord->key) {
    chordsocket = UDPWriteMessage(node->chord->ip, node->chord->port, message);
  }
  return num;
}

/******************************************************************************
 * UpgradeCurrNode()
 *
 * Arguments: node_old - estrutura com informação do nó
              KEY_new - chave do novo predecessor
              IP_new - ip do novo predecessor
              PORT_new - porto do novo predessor
 * Retorna: node - estrutura atualizada com informação do nó
 *
 * Descrição: Atualiza a informação do nó que recebe a mensagem SELF ou PRED.
              Caso receba SELF atualiza a informação sobre o seu sucessor
              Caso receba PRED atualiza a informação sobre o seu predessor
 *
 *****************************************************************************/
Node* UpgradeCurrNode(Node *node_old, int KEY_new, char IP_new[MAX], char PORT_new[MAX], int socket){

  Node* node = node_old;
  char returnmessage[MAX], newmessage[MAX];
  int n, test = 0;
  int aux_KEY = node->pred->key;
  char aux_IP[MAX],aux_PORT[MAX];
  strcpy(aux_IP, node->pred->ip);
  strcpy(aux_PORT, node->pred->port);
  if(strcmp(sender, "new") == 0){
    if(tcpSuccSocket==-1 && (node->succ->key == node->curr->key ||  node->succ->key == -1)){/*anel vazio, novo servidor sera sucessor e predecessor*/
      //atualiza a estrutura
      if (node->succ->key == node->curr->key ||  node->succ->key == -1) {
        node->succ->key = KEY_new;
        strcpy(node->succ->ip, IP_new);
        strcpy(node->succ->port, PORT_new);
      }
      //cria mensagens
      if (node->pred->key == node->curr->key ||  node->pred->key == -1) {
        test = 1;
        Alone = 1;
        sprintf(newmessage,"SELF %d %s %s\n", node->curr->key, node->curr->ip, node->curr->port);
      }
      tcpSuccSocket = socket;
      //responde ao novo sucessor
      //estabelece relacao tcp para que seja tambem o sucessor
      if (node->pred->key == node->curr->key || node->pred->key == -1) {
        node->pred->key = KEY_new;
        strcpy(node->pred->ip, IP_new);
        strcpy(node->pred->port, PORT_new);
      }
      if (test != 0) {
        close(tcpPredSocket);
        tcpPredSocket = TCPWriteMessage(IP_new, PORT_new, newmessage);
        if(tcpPredSocket==-1){
          printf("Nao foi possivel realizar a conexao TCP\n");
          return node_old;
        }
      }
    }
    else if (KEY_new != node->curr->key ) {
      sprintf(returnmessage, "PRED %d %s %s\n", KEY_new, IP_new,PORT_new);
      node->succ->key = KEY_new;
      strcpy(node->succ->ip, IP_new);
      strcpy(node->succ->port, PORT_new);
      //escreve a mensagem no antigo succ
      n=TCPWrite(tcpSuccSocket, returnmessage);
      if (n==-1) tcpSuccSocket=-1;
      //atualiza socket do sucessor
      close(tcpSuccSocket);
      tcpSuccSocket = socket;

    }else if ((sair == 0) && (node->pred->key == node->curr->key) && (KEY_new == node->curr->key)) {
      node->succ->key = KEY_new;
      strcpy(node->succ->ip, IP_new);
      strcpy(node->succ->port, PORT_new);
      close(tcpSuccSocket);
      tcpSuccSocket = -1;
    }
  } else if(strcmp(sender,"pred")==0){ // mensagem vinda do predecessor, entrada de um novo predecessor
    //cria mensagens
    if (((sair == 0) && (KEY_new != node->curr->key)) || ((sair == 0) && (node->pred->key == node->succ->key)) ) {
      sprintf(returnmessage,"SELF %d %s %s\n", node->curr->key, node->curr->ip, node->curr->port);
      //fecha socket com antigo predecessor, abre sessao com novo predecessor e envia self
      close(tcpPredSocket);
      tcpPredSocket = TCPWriteMessage(IP_new,PORT_new,returnmessage);
      if(tcpPredSocket==-1) node = succdc(node, aux_KEY, aux_IP, aux_PORT); // se der errado reestabelece com o proprio pred
      //envia mensagem ao atual predecessor avisando da mudanca de sucessor
      //atualiza estruturas
      if ((KEY_new != node->curr->key)  || ((sair == 0) && (node->pred->key == node->succ->key))) {
        node->pred->key = KEY_new;
        strcpy(node->pred->ip, IP_new);
        strcpy(node->pred->port, PORT_new);
      }
    }
  }

  return node;
}

/******************************************************************************
 * ProtocolMessage()
 *
 * Arguments: node -  estrutura com informação do nó
              message - string com a mensagem recebida
              afd - socket do nó que enviou a mensagem
 * Retorna: void
 *
 * Descrição: Trata as mensagens recebidas por UDP e TCP
 *
 *****************************************************************************/
 void ProtocolMessage(Node *node, char message[MAX], int afd){
  char command[MAX] = "\0", recieve[MAX], aux_IP[MAX] = "\0", aux_PORT[MAX] = "\0";
  char IP[MAX] = "\0";
  char PORT[MAX] = "\0";
  char* returnmessage;
  int key, i, ip = 0, port = 0;
  sscanf(message,"%s", command);
  if (strcmp(command, "SELF")==0){
    sscanf(message, "%s %d %s %s\n",command, &key, aux_IP, aux_PORT);
    strcpy(IP,aux_IP);
    strcpy(PORT,aux_PORT);
    node = UpgradeCurrNode(node, key, IP, PORT, afd);}
  else if(strcmp(command, "PRED")==0){
    sscanf(message, "%s %d %s %s\n",command, &key, aux_IP, aux_PORT);
    strcpy(IP,aux_IP);
    strcpy(PORT,aux_PORT);
    node = UpgradeCurrNode(node, key, IP, PORT, afd);}
  else if (strcmp(command, "FND")==0) {
    int k, n,num = 0;
    sscanf(message, "%s %d %d %d %s %s",command, &k, &n, &key, aux_IP, aux_PORT);
    int KEY = key;
    strcpy(IP,aux_IP);
    strcpy(PORT,aux_PORT);
    int key_result = DistanceFind(node,k);

    if (key_result == node->curr->key) {
      num = SendRSP(n,node, KEY);
      if (num==-1) {close(tcpSuccSocket); exit(1);}
      //Para a procura e inicia a resposta, foi encontrado o objeto neste nó
    } else if (key_result == node->succ->key) {
      num = TCPWrite(tcpSuccSocket, message);
      if (num==-1) {close(tcpSuccSocket); exit(1);}
      //Continua a procura no proximo nó
    } else if (key_result == node->chord->key) {
      chordsocket = UDPWriteMessage(node->chord->ip, node->chord->port, message);
    }}
  else if(strcmp(command, "RSP")==0){
    int key_destino,n, num;
    sscanf(message, "%s %d %d %d %s %s",command, &key_destino, &n, &key, aux_IP, aux_PORT);
    int key_result = DistanceFind(node,key_destino);
    if (key_destino == node->curr->key) {
      if (bentry_t == 0) {
        printf("Chave %d: nó %d %s %s\n",node->curr->key_find, key, aux_IP, aux_PORT);
      } else {
        int KEY = key;
        strcpy(IP,aux_IP);
        strcpy(PORT,aux_PORT);
        sprintf(message, "EPRED %d %s %s", KEY,IP,PORT);
        n = sendto(saveudpSocket,message,strlen(message),0, (struct sockaddr*) &addrudp,addrlenudp);
        if(n==-1) {/*error*/exit(1);}
        n=recvfrom(saveudpSocket,recieve,128,0, (struct sockaddr*)&addrudp,&addrlenudp);
        if(n==-1) /*error*/exit(1);
      }
    } else{
      if (key_result == node->succ->key) {
        num = TCPWrite(tcpSuccSocket, message);
        if (num==-1) {close(tcpSuccSocket); exit(1);}
        //Continua a procura no proximo nó
      } else if (key_result == node->chord->key) {
        chordsocket = UDPWriteMessage(node->chord->ip, node->chord->port, message);
      }
    }}
  else if(strcmp(command, "EFND")==0){
    int key_find;
    bentry_t = 1;
    efnd = 1;
    sscanf(message, "%s %d",command, &key_find);
    node = find(node, key_find);
    if (efnd == 0) {
      int n = 0;
      static int count = 0;
      sprintf(message, "EPRED %d %s %s", node->curr->key, node->curr->ip, node->curr->port);
      n = sendto(saveudpSocket,message,strlen(message),0, (struct sockaddr*) &addrudp,addrlenudp);
      if(n==-1) {/*error*/exit(1);}
    }
    efnd = 0;
}
  else if(strcmp(command, "EPRED")==0){
    if (count == 0) {
      sscanf(message, "%s %d %s %s",command, &key, aux_IP, aux_PORT);
      strcpy(IP,aux_IP);
      strcpy(PORT,aux_PORT);
      if (strlen(PORT)>5) {
        PORT[5] = '\0';
      }
      node = new(node,node->curr->key,node->curr->ip, node->curr->port);
      printf("\nO nó %d é predecessor de ip %s e porto %s\n", key, IP, PORT);
      node = pentry(node, key, IP, PORT);
      close(udpsocket);
      udpsocket = -1;
    }
    if (newring == 1) {
      count++;
    }
  }
  return;
}

/******************************************************************************
 * OpenServer()
 *
 * Arguments: argv - vetor que contém os argumentos inseridos da chamada do programa
              node - estrutura com informação do nó
 * Retorna: void
 *
 * Descrição: Abre os servidores UDP e TCP para receber mensagens da interface e
              recebe as mensagens enviadas por TCP e UDP do predecessor e de nós fora
              do anel.
 *
 *****************************************************************************/
 void OpenServer(char *argv[], Node *node){

//---------------------------------Inicializar Variaveis Globais---------------------------------//
  Alone = 0;
  sair = 0;
  bentry_t = 0;
  efnd = 0;
  newring = 0;
  count = 0;
//---------------------------------Inicializar Variaveis Globais---------------------------------//

//-------------------------------------------Variaveis-------------------------------------------//
  struct addrinfo hints,*res;
  struct sockaddr addr; socklen_t addrlen;
  struct timeval timeout;
  enum {idle,busy} state;
  int fd_tcp,newfd,errcode, fd_udp, counter, KEY, N,nr, ip = 0, port = 0, afd = -1, ntimeouts = 0;
  ssize_t nbytes,n;
  char *ptr, buffer[1024], bufferudp[1024], incompletebuffer[MAX]=" ", message[MAX]=" ";
  char IP[MAX];
  char PORT[MAX];
  char *PORT_aux = NULL;
  char *IP_aux = NULL;
  fd_set rfds;
//-------------------------------------------Variaveis-------------------------------------------//


//-----------------------------------Inicalizar o nó e socket-----------------------------------//
  node = Init_node(node);
  strcpy(IP,argv[2]);
  strcpy(PORT,argv[3]);
  KEY = strtol(argv[1], &ptr, 10);
  if ((KEY > 32) || (KEY < 0)) {
    printf("**************************************************** \n\n");
    printf("   A chave do nó deve estar entre 0 e 32\n\n");
    printf("**************************************************** \n\n");
    FreeNode(node);
    freeaddrinfo(res);
    return;
  }
  if ((atoi(PORT) > 58111) || (atoi(PORT) < 58000)) {
    printf("**************************************************** \n\n");
    printf("   O porto tem que estar entre 58000 e 58111\n\n");
    printf("**************************************************** \n\n");
    FreeNode(node);
    freeaddrinfo(res);
    return;
  }
  /*ABERTURA DO SOCKET TCP*/
  if((fd_tcp=socket(AF_INET,SOCK_STREAM,0))==-1)exit(1);//r
  memset(&hints,0,sizeof hints);
  hints.ai_family=AF_INET;//IPv4
  hints.ai_socktype=SOCK_STREAM;//TCP socket
  hints.ai_flags=AI_PASSIVE;


  if((errcode=getaddrinfo(NULL,PORT,&hints,&res))!=0)/*r*/exit(1);
  if(bind(fd_tcp,res->ai_addr,res->ai_addrlen)==-1)/*r*/exit(1);
  if(listen(fd_tcp,5)==-1)/*r*/exit(1);

  /*ABERTURA DO SOCKET UDP*/
  if((fd_udp=socket(AF_INET,SOCK_DGRAM,0))==-1)exit(1);//error
  memset(&hints,0,sizeof hints);
  hints.ai_family=AF_INET;//IPv4
  hints.ai_socktype=SOCK_DGRAM;//UDP socket
  hints.ai_flags=AI_PASSIVE;
  if((errcode=getaddrinfo(NULL,PORT,&hints,&res))!=0)/*error*/exit(1);
  if(bind(fd_udp,res->ai_addr,res->ai_addrlen)==-1)/*error*/exit(1);
//-----------------------------------Inicalizar o nó e socket-----------------------------------//

  udpsocket = -1;
  chordsocket = -1;
  state=idle;
//-----------------------------------Ciclo para realizar os modos e receber as mensagens-----------------------------------//
  while(1){
    FD_ZERO(&rfds);
    //no inicio só é possivel receber mensagens da interface
    FD_SET(0, &rfds);
    FD_SET(fd_tcp, &rfds);
    FD_SET(fd_udp, &rfds);
    maxfd=max(fd_tcp,fd_udp);

    if(state == busy){
      //ja existe um anel portanto passa a poder receber mensagens tcp
      FD_SET(afd,&rfds); maxfd=max(maxfd,afd);
    }
    if(tcpPredSocket!=-1){
      //pode receber mensagens do canal ja estabelecido para o predecessor
      FD_SET(tcpPredSocket, &rfds); maxfd=max(maxfd,tcpPredSocket);
    }
    if(tcpSuccSocket!=-1){
      //pode receber mensagens do canal ja estabelecido para o sucessor
      FD_SET(tcpSuccSocket, &rfds); maxfd=max(maxfd,tcpSuccSocket);
    }

    if (udpsocket != -1) {
      //pode receber mensagens do canal udp ja estabelecido
      FD_SET(udpsocket, &rfds); maxfd=max(maxfd,udpsocket);
    }

    if (chordsocket != -1) {
      //pode receber mensagens do canal udp ja estabelecido
      FD_SET(chordsocket, &rfds); maxfd=max(maxfd,chordsocket);
    }

    counter=select(maxfd+1,&rfds,(fd_set*)NULL,(fd_set*)NULL,(struct timeval *)NULL);
    if(counter<0){fprintf(stderr,"error: %s\n",strerror(errno));exit(1);}

if((node->curr->key != -1) &&  FD_ISSET(fd_tcp,&rfds)){
  addrlen=sizeof(addr);
  if((newfd=accept(fd_tcp, (struct sockaddr*)&addr, &addrlen))==-1) exit(1);
  switch(state){
    case idle:
    afd=newfd;
    state=busy;
    maxfd=max(maxfd,afd);
    break;

    case busy:
    write(newfd, "Busy\n", 5);
    close(newfd);
  }
}

if(FD_ISSET(0,&rfds)){
  node = interface(node, KEY, IP, PORT);
  if(flag==1) break; //comando exit
  if(flag==0) continue;
  if(flag==2) { // comando leave
    if(node->pred->key!=-1){close(afd);
      state=idle;}
      continue;
    }
    if ((flag == 2) && (newring == 1)) {
      node = Init_node(node);
      close(afd);
      afd = -1;
      close(newfd);
      newfd = -1;
      close(udpsocket);
      udpsocket = -1;
      close(tcpPredSocket);
      tcpPredSocket = -1;
      maxfd=max(fd_tcp,fd_udp);
      if (node->chord->key != -1) {
        node->chord->key = -1;
        close(chordsocket);
        chordsocket = -1;
      }
    }
    continue;
  }
  //-----------------------------------Fecha o resto das socket e atualiza a estrutura depois do comando leave-----------------------------------//
  if (flag == 2 && newring != 1) {
    node = Init_node(node);
    close(afd);
    afd = -1;
    close(newfd);
    newfd = -1;
    close(udpsocket);
    udpsocket = -1;
    close(tcpPredSocket);
    tcpPredSocket = -1;
    maxfd=max(fd_tcp,fd_udp);
    if (node->chord->key != -1) {
      node->chord->key = -1;
      close(chordsocket);
      chordsocket = -1;
    }
  }
  //-----------------------------------------------------------------------------------------------------------------------------------------------//

  if(FD_ISSET(afd,&rfds)){
    strcpy(buffer,"\0");
    nbytes=sizeof(buffer);/*receber instrucoes dos outros servidores*/
    if((n = read(afd,buffer,nbytes))!=0){
      if (n==-1) {/*error*/exit(1);}
     buffer[n]='\0';
      if(buffer[n-1]!='\n'){
        if(strcmp(incompletebuffer, " ")!=0){
          strcat(incompletebuffer,buffer);
        }
        strcpy(incompletebuffer, buffer);
      }
      else{
        if(strcmp(incompletebuffer, " ")!=0){ //caso haja uma parte a espera de fim
          strcat(incompletebuffer,buffer);
          strcpy(buffer, incompletebuffer);
          strcpy(incompletebuffer, " ");
        }
      }
        strcpy(sender,"new"); // mensagem recebida por um novo servidor que nao pertence ao anel
        if (Alone == 0) {
          ProtocolMessage(node, buffer, afd);
          state=idle;
        } else{
          ProtocolMessage(node, buffer, afd);
          state=idle;
        }
        continue;
      }
      else{
        close(afd);
        state=idle;
      }
    }
    if(FD_ISSET(tcpPredSocket,&rfds)){
      strcpy(buffer,"\0");
      nbytes=sizeof(buffer);
      if((n = read(tcpPredSocket,buffer,nbytes))!=0){
        if (n==-1) {/*error*/exit(1);}
        buffer[n]='\0';
        if(buffer[n-1]!='\n'){
          if(strcmp(incompletebuffer, " ")!=0){
            strcat(incompletebuffer,buffer);
          }
          strcpy(incompletebuffer, buffer);
        }
        else{
          if(strcmp(incompletebuffer, " ")!=0){
            strcat(incompletebuffer,buffer);
            strcpy(buffer, incompletebuffer);
            strcpy(incompletebuffer, " ");
          }
        }
          strcpy(sender,"pred"); //mensagem recebida do predecessor
          ProtocolMessage(node, buffer, tcpPredSocket);
          maxfd = max(maxfd,tcpPredSocket);
          continue;
      }
      else tcpPredSocket=-1; //recebeu a mensagem de que foi desconectado
    }

    if(FD_ISSET(fd_udp, &rfds)){ // mensagem recebida por novo canal udp
      addrlenudp=sizeof(addrudp);
      n=recvfrom(fd_udp,bufferudp,128,0, (struct sockaddr*)&addrudp,&addrlenudp);
      saveudpSocket = fd_udp;
      bufferudp[n] = '\n';
      if(n==-1) {/*error*/exit(1);}
      //nao pode receber entry se nao houver anel
      if (node->curr->key==-1){
        n=sendto(fd_udp,"No ring yet\n",strlen("No ring yet\n"),0, (struct sockaddr*) &addrudp,addrlenudp);
        continue;
      }
      udpsocket = fd_udp;
      n=sendto(fd_udp,"ACK",strlen("ACK"),0, (struct sockaddr*) &addrudp,addrlenudp);
      if(n==-1) {/*error*/exit(1);}
      strcpy(sender,"udp"); //mensagem recebida do predecessor
      ProtocolMessage(node, bufferudp, fd_udp);
    }
    else if(FD_ISSET(udpsocket, &rfds)){ //recebe mensagem por canal udp ja aberto
      addrlen=sizeof(addr);
      n=recvfrom(udpsocket,bufferudp,128,0, (struct sockaddr*)&addr,&addrlen);
      if(n==-1) {/*error*/exit(1);}
      if(strcmp(bufferudp, "No ring yet\n")==0) printf("Servidor contactado nao faz parte de um anel\n");
      else{
        if (strcmp(buffer, "ACK") != 0) {
          n=sendto(udpsocket,"ACK",strlen("ACK"),0, (struct sockaddr*) &addr,addrlen);
          if(n==-1){ /*error*/;exit(1);}
        }
        ProtocolMessage(node, bufferudp, udpsocket);
      }
    }
  }//while(1)
//-----------------------------------Ciclo para realizar os modos e receber as mensagens-----------------------------------//
  FreeNode(node);
  freeaddrinfo(res);
  close(maxfd);
  close(afd);
  close(fd_tcp);
  close(fd_udp);
  return;
}
