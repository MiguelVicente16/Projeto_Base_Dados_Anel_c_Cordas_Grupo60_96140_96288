#include <sys/types.h>
#include <sys/socket.h>
#define MAX 1000
typedef struct Node_1 Node_1;

typedef struct Node Node;


extern int tcpSuccSocket;
extern int tcpPredSocket;
extern int tcpSocket;
extern int udpsocket;
extern int saveudpSocket;
extern int chordsocket;
extern char sender[MAX];
extern int efnd;
extern socklen_t addrlenudp;
extern struct sockaddr_in addrudp;
extern int flag;
extern int print;
extern int maxfd;
extern int bentry_t;
extern int Alone;
extern int sair;
extern int newring;
extern int count;
void OpenServer(char *argv[], Node *node);
void ProtocolMessage(Node *node_old, char *message, int afd);
Node* UpgradeCurrNode(Node *node_old, int KEY_new, char IP_new[], char PORT_new[], int socket);
int SendRSP(int n, Node* node, int KEY);
