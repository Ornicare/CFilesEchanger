char * toLowerCase(char * word);
int waitingForServerMessage(int sock, char* commande);
int download(char *fichier,int socket);
int upload(char *fichier,int socket);
int file_exists(char * filename);
int min(int a, int b);
int sendCommandToServer(int sock, char* commande);
int parseCommand(int sock, char* mesg);
void clean_stdin(void);
int getLock(char * filePath);
int setLock(char * filePath);
int unLock(char * filePath);

int socket_descriptor; /* descripteur de socket */
