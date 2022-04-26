typedef struct Node_1 Node_1;

typedef struct Node Node;

extern int tcpSuccSocket;
extern int tcpPredSocket;
extern int udpsocket;
extern int chordsocket;
extern int flag;
Node *Init_node(Node *node);
void PrintStatment(int num);
void FreeNode(Node *node);
int DistanceFind(Node *,int );
