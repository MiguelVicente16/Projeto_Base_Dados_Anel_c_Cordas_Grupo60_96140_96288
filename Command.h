
typedef struct Node_1 Node_1;

typedef struct Node Node;


extern int tcpSuccSocket;
extern int tcpPredSocket;
extern int tcpSocket;
extern int udpsocket;
extern int chordsocket;
extern int flag;
extern int maxfd;
extern int sair;
extern int efnd;

int TCPWrite(int fd, char* str);

int TCPWriteMessage(char IP[], char PORT[], char *message);

int UDPWriteMessage(char *IP, char *PORT, char *message);

Node *bentry(Node *node, int, int KEY, char *IP, char *PORT);

Node *chord(Node *node, int key_chord, char *IP_chord, char *PORT_chord);

Node *find(Node *node, int KEY);

Node *leave(Node *node);

Node * pentry(Node* , int key_pred, char IP_pred[], char PORT_pred[]);

Node* new(Node* node, int KEY, char *IP, char *PORT);

void show(Node *node);

Node *succdc(Node *node, int KEY, char * IP, char * PORT);
