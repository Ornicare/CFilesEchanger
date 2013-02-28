int parseCommand(char* BUFFER, int BUFFER_LENGTH, int sock);
int cdDown(int sock,char* argument);
void list(int sock);
void pwd(int sock);
char* pgetcwd(void); //Récupère le dossier courant (dans lequel le pgrm est lancé).
void stop(int sock);
int upload(char* fichier, int socket);
int min(int a, int b);
int uploadManager(int sock, char* argument);
void *talker( void *d );
int waitingForClientMessage(int socket,char* commande);
void listByType(int sock, int type, DIR* rep);
int file_exists(char * filename);
int dir_exists(char * dirpath);

//contain thread paths.
char* currentPaths[5000];
int static socket_descriptor;

enum {
    FIRST = 1,
    FACTOR = 2
};
