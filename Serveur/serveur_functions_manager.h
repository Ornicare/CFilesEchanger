int parseCommand(char* BUFFER, int BUFFER_LENGTH);
int help();
int download(char* fichier);
int cdDown(char* argument);
int sendCommandToClient(char * message);
void list();
void pwd();
//void cdUp();
char* pgetcwd(void); //Récupère le dossier courant (dans lequel le pgrm est lancé).
void stop();
int upload(char* fichier);
int min(int a, int b);
int uploadManager(char* argument);
void *talker( void *d );
int waitingForClientMessage(char* commande);
void listByType(int type, DIR* rep);
int file_exists(char * filename);
int dir_exists(char * dirpath);
int getLock(char * filePath);
int setLock(char * filePath);
int unLock(char * filePath);

int getLocked(char * filePath);
int setLocked(char * filePath);
int unLocked(char * filePath);


int static socket_descriptor;

//global to each thread
static __thread int tls_sock;
static __thread char currentPath[2000];

//Can download 10000 files atthe same time
char * lockedFiles[10000];

enum {
    FIRST = 1,
    FACTOR = 2
};
