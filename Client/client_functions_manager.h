int download(char *fichier,int socket);
int file_exists(char * filename);
int min(int a, int b);
int sendCommandToServer(int sock, char* commande);
int parseCommand(int sock, char* mesg);
void clean_stdin(void);

int socket_descriptor; /* descripteur de socket */
