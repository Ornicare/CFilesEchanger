#include <stdio.h>
#include <stdlib.h>
#include <linux/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
//pour list()
#include <sys/types.h>
#include <dirent.h>
//pour pwd()
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include "serveur_functions_manager.h"
#include "md5manager.h"

#include "crc.h"

#define TAILLE_MAX_NOM 256
#define PACKET_SIZE 1 //pas plus, ça passe pas
#define DEBUG 1



/**************************************/

/*
 * Fonction de prise en charge des threads 
 */
void *talker( void *d ) {

	printf("&&&&&&&&&&&&&&&&&&&&&%i\n",pthread_self());

	//static __thread int p = 10;
	
	//__thread int iqssqqqs = 42;
	char buffer[256];
	char temp[256];
	//
	int longueur;
	//int sock;
	//printf("@@@@@@@@@@@@@@@@@@%i\n", p);
	
	
	char asciiCode[2]="1";

	tls_sock = (int)d; //bah, faut bien récupérer l'id du socket... on cast void* en int...
	
	strncpy(currentPath,pgetcwd(),strlen(pgetcwd())); //initialisation du chemin courant.
	
	if(strlen(currentPath)>1 && currentPath[strlen(currentPath)-1]!='/')
	{
		strcat(currentPath,"/\0");	
	}

	while(1)
	{
		printf("[%i]Attente d'un message.\n",tls_sock);
		//do
		//{
			//sleep(30);
			
			//printf("%i@@@@@@@@@@@@@@@@@@%s\n",sock,tls_i);
			longueur = read(tls_sock, buffer, sizeof(buffer)); printf("READ : @%s@%d\n",buffer,buffer[0]);
			
			//tls_i = buffer[0];
			//strncpy(tls_i,buffer,strlen(buffer));
			
			//sprintf(asciiCode,"%d",buffer[0]);
		//} while(strcmp(asciiCode,"0")==0);
		//buffer[longueur] = '\0';
		//if(strcmp(asciiCode,"0")!=0)
		//{
			if(longueur <= 0) return;  //si la lecture échoue, on annule tout.
		
			write(tls_sock, "RECEIVED\0",9);  if(DEBUG) printf("WRITE : %s\n","RECEIVED\0");
		
			//
			read(tls_sock, temp, sizeof(temp)); if(DEBUG) printf("READ : %s\n",temp);
		
		
		
			if(parseCommand(buffer, sizeof(buffer)))
			{
				printf("[%i]Une erreur est survenue, fermeture du socket.\n",tls_sock);	
				break;
			}
			else
			{
				//printf("%s\n", buffer);
				//printf("[%i]Commande réussie : %s\n",sock,buffer);
				write(tls_sock, "ENDSTREAM\0", 10); if(DEBUG) printf("WRITE : %s\n","ENDSTREAM\0");
			}
		//}
	}

	close(tls_sock); //on ferme le socket.
}

/*
 * Fonction d'analyse du message envoyé par le client.
 */
int parseCommand(char* BUFFER, int BUFFER_LENGTH) { //     /!\utiliser fork ou mutex
	
	/*
	 * Préformatage des chaines.
	 */
	int k=0;
	//printf("%i@@@@@@@@@@@@@@@@@@%s\n",sock,tls_i);
	printf("sffddf\n");
	while(BUFFER[k]!=' ' && BUFFER[k]!='\0')
	{
		k++;
	}
	BUFFER[k]='\0';
	k++;
	printf("sffddf\n");
	
	
	if(strcmp(BUFFER,"LIST")==0) //Liste le répertoire courant.
	{
		list();
	}
	else if(strcmp(BUFFER,"PWD")==0) //récupère le répertoire courant.
	{
		pwd();
	}
	else if(strcmp(BUFFER,"STOP")==0) //si le message envoyé est "STOP", on quitte tout.
	{
		stop();
	}
	else if(strcmp(BUFFER,"GET")==0) 
	{
		char argument[256];
		int temp_k = k;
		while(BUFFER[k]!='\0' && k<255)
		{
			argument[k-temp_k]=BUFFER[k];
			k++;
		}
		argument[k-temp_k]='\0';
		uploadManager(argument);
	}
	else if(strcmp(BUFFER,"CDDOWN")==0) 
	{
		char argument[256];
		int temp_k = k;
		while(BUFFER[k]!='\0' && k<255)
		{
			argument[k-temp_k]=BUFFER[k];
			k++;
		}
		argument[k-temp_k]='\0';
		cdDown(argument);
	}
	else if(strcmp(BUFFER,"CDUP")==0) 
	{
		cdUp();
	}
	else if(strcmp(BUFFER,"MKDIR")==0) 
	{
		char argument[256];
		int temp_k = k;
		while(BUFFER[k]!='\0' && k<255)
		{
			argument[k-temp_k]=BUFFER[k];
			k++;
		}
		argument[k-temp_k]='\0';
		
		char filePath[1024];
		memset( filePath, '\0', sizeof(filePath));
	
		strncpy(filePath,currentPath,strlen(currentPath));
		strcat(filePath,argument);
		
		mkdir(filePath,0777);
	}
	else if(strcmp(BUFFER,"HELP")==0) 
	{
		help();
	}
	else if(strcmp(BUFFER,"PUSH")==0)
	{
		char argument[256];
		int temp_k = k;
		while(BUFFER[k]!='\0' && k<255)
		{
			argument[k-temp_k]=BUFFER[k];
			k++;
		}
		argument[k-temp_k]='\0';
		
		char filePath[1024];
		memset( filePath, '\0', sizeof(filePath));
	
		strncpy(filePath,currentPath,strlen(currentPath));
		strcat(filePath,argument);
		
		char temp[256];
		
		write(tls_sock,"SYNCHRO\0",8);  if(DEBUG) printf("WRITE : %s : %s\n","SYNCHRO\0",filePath);
		
		read(tls_sock,temp,sizeof(temp)); if(DEBUG) printf("READ : %s\n",temp);
		if(strcmp(temp,"CONTINUE\0")!=0) return 0; //Client abort
			
		char fileAvailable[256];
		
		if(getLocked(filePath)!=0)
		{
			write(tls_sock,"ERR:FILE_ALREADY_IN_USE\0",24);  if(DEBUG) printf("WRITE : %s : %s\n","ERR:FILE_ALREADY_IN_USE\0",filePath);
			
			//Wait for client message.
			
			read(tls_sock,temp,sizeof(temp)); if(DEBUG) printf("READ : %s\n",temp);
			return 0;
		}
		
		
		
		if(file_exists(filePath))
		{	
			
			//lock the file
			setLocked(filePath);
			
			
			write(tls_sock,"ERR:FILE_ALREADY_EXISTS\0",24);  if(DEBUG) printf("WRITE : %s : %s\n","ERR:FILE_ALREADY_EXISTS\0",filePath);
			
			//Wait for client message.
			
			read(tls_sock,temp,sizeof(temp)); if(DEBUG) printf("READ : %s\n",temp);
			if(strcmp(temp,"ABORT")==0)
			{
				unLocked(filePath);
				return 0;
			}
			
			remove(argument);

		}
		else
		{
			write(tls_sock,"ALL_OK\0",7);  if(DEBUG) printf("WRITE : %s\n","ALL_OK\0");
			
			//Wait for client message.
			read(tls_sock,temp,sizeof(temp)); if(DEBUG) printf("READ : %s\n",temp);
		
		}
		
		
		
		printf("Dowloading the file\n");
		download(filePath);
		unLocked(filePath);
		
	}
	return 0;
}

int help()
{
	char * help = "CDUP: go to the parent directory\nCDDOWN <directory>: go to <directory>\nPWD: get the current directory\nLIST: get files and directories in the current directory\nGET <file>: try to download <file>\nPUSH <file>: try to push <file> on the working directory on the server.\n";
	write(tls_sock,help, strlen(help));
	waitingForClientMessage("CONTINUE");
	return 0;
}

int cdDown(char* argument)
{
	char testPath[255];
	strncpy(testPath,currentPath,strlen(currentPath));
	testPath[strlen(currentPath)]='\0';
	
	//Rajouter un / à la fin s'il n'y est pas
	if(strlen(argument)>1 && argument[strlen(argument)-1]!='/')
	{
		printf("DEBUG : %c\n",argument[strlen(argument)-1]);
		strcat(argument,"/\0");	
	}
	
	//sprintf(testPath, "%s%s", currentPath, argument); 
	//strcat(testPath,currentPath);
	strcat(testPath,argument);
	printf("DEBUG : %s\n",testPath);
	printf("DEBUG : %s\n",currentPath);
	if(dir_exists(testPath)) strncpy(currentPath,testPath,strlen(testPath));
	printf("DEBUG : %s\n",currentPath);
	write(tls_sock,currentPath,strlen(currentPath));
	if(waitingForClientMessage("CONTINUE")) return 1;
	
	//chdir(currentPath);
	return 0;
}

int cdUp()
{
	if(strcmp(currentPath,"/")==0) return 0; //on est déjà à la racine.
	char buffer[strlen(currentPath)+1];
	strncpy(buffer, currentPath, strlen(currentPath));
	int k=strlen(currentPath)-1;
	do
	{
		buffer[k]='\0';
		k--;
	} while(buffer[k]!='/');
	buffer[strlen(currentPath)]='\0';
	strncpy(currentPath,buffer,sizeof(buffer));
	write(tls_sock,buffer,sizeof(buffer));
	if(waitingForClientMessage("CONTINUE")) return 1;
	
	printf("@%s@\n",currentPath);
	//chdir(currentPath); //choisit le dossier courant.
	return 0;
}

/*
 * Liste les fichiers et dossiers du répertoire courant (chdir()=>choisir ce répertoire)
 */
void list() {
	struct dirent *lecture;
	DIR *rep;
	printf("qdsqdsq\n");
	rep = opendir(currentPath);
	printf("%s",currentPath);
	write(tls_sock,"\n**Dossiers : \n\0",16);
	waitingForClientMessage("CONTINUE");
	printf("**Dossiers : \n");
	listByType(DT_DIR,rep);
	
	rep = opendir(currentPath );
	write(tls_sock,"\n**Fichiers : \n\0",16);
	waitingForClientMessage("CONTINUE");
	printf("**Fichiers : \n");
	listByType(DT_REG,rep);
}

/*
 * type peut prendre les valeurs suivantes.
 *      DT_BLK      This is a block device.
 *      DT_CHR      This is a character device.
 *      DT_DIR      This is a directory.
 *      DT_FIFO     This is a named pipe (FIFO).
 *      DT_LNK      This is a symbolic link.
 *      DT_REG      This is a regular file.
 *      DT_SOCK     This is a UNIX domain socket.
 *      DT_UNKNOWN  The file type is unknown
 */
void listByType(int type, DIR* rep) {
	char buffer[256];
	struct dirent *lecture;
	while ((lecture = readdir(rep))) {
		if(strcmp(lecture->d_name, ".") && strcmp(lecture->d_name, ".."))
	      	{
			if(lecture->d_type == type)
		 	{
		 		strncpy(buffer,lecture->d_name,strlen(lecture->d_name));
		 		buffer[strlen(lecture->d_name)]='\0';
		    		write(tls_sock,buffer,strlen(lecture->d_name)+1);
				printf("%s\n", lecture->d_name, lecture->d_type);
				waitingForClientMessage("CONTINUE");
		 	}
	      	}
	}
	closedir(rep);
}

/*
 * Renvoie le répertoire courant à l'utilisateur.
 */
void pwd() {
	write(tls_sock, currentPath, strlen(currentPath));
	waitingForClientMessage("CONTINUE");
}


/*
 * Récupère le répertoire courant du programme
 */
char* pgetcwd(void)
{
	char *buf;
	char *newbuf;
	size_t size;

	size = FIRST;
	if ((buf = malloc(size)) == NULL) {
		errno = ENOMEM;
		return NULL;
	}
	while (getcwd(buf, size) == NULL) {
		if (errno != ERANGE) {
			free(buf);
			return NULL;
		}
		if ((newbuf = realloc(buf, size * FACTOR)) == NULL) {
			free(buf);
			errno = ENOMEM;
			return NULL;
		}
		buf = newbuf;
		size *= FACTOR;
	}
	return buf;
}

/*
 * Attention aux priorités à droite.
 */
void stop() {
	printf("[%i]Fermeture du lien serveur-client.\n",tls_sock);
	char* message = "Fermeture du lien serveur-client...\0";
	//write(sock,message,strlen(message)); printf("WRITE : %s\n",message);
	close(tls_sock);
	pthread_exit();
	//close(socket_descriptor);
	//exit(1);
}

/*
 * La fonction min
 */
int min(int a, int b)
{
	int tmp = a;
	if(a>b) tmp=b;
	return tmp;
}

/*
 * Cette fonction parsera le GET et essayera d'envoyer le fichier à l'utilisateur.
 */
int uploadManager(char* argument)
{
	//Attention variable conservée entre deux appels ! (D'où le memset)
	char filePath[1024];
	memset( filePath, '\0', sizeof(filePath));
	
	strncpy(filePath,currentPath,strlen(currentPath));
	strcat(filePath,argument);
	
	//Attention get lock avant le file_exist (fait sauter le lock)
	printf("@@@@@@@@@@@@@@@@@@@@%i\n",getLock(filePath));
	if(getLock(filePath) || getLocked(filePath))
	{
		write(tls_sock,"ERR:FILE_ALREADY_IN_USE_OR_NOT_EXISTS\0",39);  if(DEBUG) printf("WRITE : %s : %s\n","ERR:FILE_ALREADY_IN_USE_OR_NOT_EXISTS\0",filePath);
		char temp[256];
		read(tls_sock,temp,sizeof(temp)); if(DEBUG) printf("READ : %s\n","temp");
		return 1;
	}

	
	if(!file_exists(filePath))
	{	
		write(tls_sock,"ERR:FILE_NOT_EXISTS\0",20);  if(DEBUG) printf("WRITE : %s : %s\n","ERR:FILE_NOT_EXISTS\0",filePath);
		char temp[256];
		read(tls_sock,temp,sizeof(temp)); if(DEBUG) printf("READ : %s\n","temp");
		return 1;
	}
	
	//get the lock
	setLocked(filePath);
	
	
	//setLock(filePath);
	//printf("@@@@@@@@@@@@@@@@@@@@%i\n",getLock(filePath));
	write(tls_sock,"FILE_EXIST\0",11);  if(DEBUG) printf("WRITE : %s\n","FILE_EXIST\0");
	
	
	//char temp[256];
	//read(sock,temp,sizeof(temp));
	
	printf("Début de l'upload\n");
	if(upload(filePath)) return 1;
	return 0;
	
}

/*
 *		read("START")
 *			|
 *		write(taille du fichier)
 *			|
 *		read("CONTINUE")
 *			|
 *		write(md5 du fichier)
 *			|
 *		read("CONTINUE")
 *			|
 *		do
 *		{
 *			write(un morceau du fichier)
 *		}
 *		while(!read("CONTINUE")) //tant que le client n'a pas reçu le paquet correctement (vérification via son CRC), le serveur lui renvoie. 
 */
int upload(char* fichier)
{
	//Initialisation du crc
	crcInit();
	
	/*
	 * Pour les tests, simulation de paquets corrompus.
	 */
	//Initialisation de random (on change le seed en fonction de l'heure)
	srand(time(NULL));
	float r; //notre valeur aléatoire.
	float o; //notre octet corrompu
	char old_char; //la bonne valeur.
	int old_char_index;
	
	/*
	 * Déclarations
	 */
	FILE *fs = fopen(fichier, "rb"); //Le fichier à envoyer. //rb ouverture du fichier en lecture binaire
	
	//remettre le lock
	printf("0=Lock placé : %i\n",setLock(fichier));
	
	//int result = flock(fileno(fs), 2);
	//printf("Lock : %i\n", result);
	int octet_envoye; //Pour vérifier que l'envoi s'est bien fait.
	int taille_fichier;
	unsigned char *buffer; //Le paquet qui va être envoyé
	unsigned char *morceaufichier; //Morceau de fichier mit dans buffer.
	//allocations mémoire
	morceaufichier = malloc((PACKET_SIZE+1) * sizeof(char));
	buffer = malloc((PACKET_SIZE+1+5) * sizeof(char)); //buff sera la taille d'un fragment de fichier + le crc.
	
	//indicateurs de lecture
	int taille_lue;
	int taille_temporaire;
	//deux trois trucs
	char tailleFichier[100]; //pour gérer la taille du fichier
	char commandReader[10]; //reception des commandes serveur.
	char bufferT[33]; //stockera le md5 du fichier.
	char crcPaquetServeur[5]; //stockera le CRC du morceau de fichier (mis au début de buffer)
	int current_paquet_size; //taille du paquet courant.
	int nb_paquets = 0; //le nombre de paquets corrects envoyés.
	//gestion de la progress bar
	int percent = 0;
	int old_percent = 0;
	//Pour le calcul.
	int k;
	
	//Calcul de la taille du fichier.
	fseek(fs, 0, SEEK_END); //déplace le curseur de lecture du fichier de 0 octets par rapport à la fin du fichier
	taille_fichier =  ftell(fs); //ftell : position actuelle du curseur.
	

	//Initialisation des indicateurs de lecture.
	fseek(fs, 0, SEEK_SET); //on place le curseur au début.
	taille_temporaire = ftell(fs);
	taille_lue=0;
	
	//On met la taille dans un buffer
	sprintf(tailleFichier, "%i", taille_fichier);
	
	//On attend START de la part du client.
	if(waitingForClientMessage("START"))
	{
		unLocked(fichier);
		return 1;
	}
	//envoi de la taille du fichier au client
	write(tls_sock, tailleFichier, sizeof(tailleFichier)); if(DEBUG) printf("WRITE : %s\n",tailleFichier);

	//réception de CONTINUE.
	if(waitingForClientMessage("CONTINUE"))
	{
		unLocked(fichier);
		return 1;
	}
	//envoi du md5 du fichier
	
	getMD5checkSum(fichier, bufferT); //md5 du fichier envoyé.
	write(tls_sock, bufferT, sizeof(bufferT));  if(DEBUG) printf("WRITE : %s\n",bufferT);

	if(waitingForClientMessage("CONTINUE"))
	{
		unLocked(fichier);
		return 1;
	}
	

	printf("__________________________________________________\n");
	
	while  (taille_lue<taille_fichier)
	{
		nb_paquets+=1; //On compte le nombre de paquets
		current_paquet_size=min(PACKET_SIZE,taille_fichier-taille_lue); //la taille du paquet courant
		fread(morceaufichier,PACKET_SIZE,1,fs); //lecture du bloc de fichier.
		
		 
		//morceaufichier[current_paquet_size]='\0'; //au cas où...
		//buffer[4]='\0';
		//printf("%s\n",buffer);
		//calcul du crc du morceau de fichier.	
		//sprintf(buffer, "%X", crcFast(morceaufichier, min(PACKET_SIZE,taille_fichier-taille_lue))); //mettre le crc au début du paquet.
		
		//On met le moreceau de fichier dans le buffer.
		/*for(k = 0;k<(PACKET_SIZE);k++)
		{
			buffer[4+k]=morceaufichier[k];
		}
		buffer[PACKET_SIZE+5]='\0';
		*/
		
		/*
		 * Pour les tests
		 */
		//corruption du paquet
		/*
		r = (float)(rand()) / (float)(RAND_MAX);
		if(r < 0.001)
		{
			//printf("Erreur !\n");
			o=(float)(rand()) / (float)(RAND_MAX);
			old_char_index = (int)((PACKET_SIZE+1+5)*o);
			old_char = buffer[old_char_index];
			buffer[old_char_index]='A';
		}
		*/
		/*
		 * Fin du code de test.
		 */
		 
	
		

		//commandReader[1]='A'; //Pour annule. (initialisation)
		//do {
			//printf("@@@@@@@@@@@%s@@@@@@@@@@\n",buffer);
			
			octet_envoye=write(tls_sock, morceaufichier, min(PACKET_SIZE,taille_fichier-taille_lue));
			
			//code de test
			/*buffer[old_char_index]=old_char;*/
			
			//lecture de la réponse du client.
			//octet_envoye=read(socket, commandReader, sizeof(commandReader));
		//} while (strcmp(commandReader,"CONTINUE")!=0 || octet_envoye<0); //Si le client n'a pas reçu ou si y'a une erreur d'envoi
		
		/*
		 * Gestion de la progress bar.
		 */
		percent = (int)((50*(double)taille_lue)/((double)taille_fichier));
		if(percent>old_percent)
		{
			for(k = old_percent;k<percent;k++)
			{
				printf("=");
				fflush(stdout); //On force stdout à se rafraichir.
			}

			old_percent = percent;
		}

		taille_lue+=PACKET_SIZE; //Pour calculer la taille du paquet courant.
		//taille_temporaire=ftell(fs); //Pour savoir ou l'on en est dans la lecture du fichier.
	}
	for(k = percent;k<50;k++)
	{
		printf("=");
		fflush(stdout); //On force stdout à se rafraichir.
	}
	printf("\n"); //(Juste pour terminer la progress bar)
	free(morceaufichier);
	free(buffer);
	
	//unlock the file
	unLock(fichier);
	unLocked(fichier);

	return 0;
}

int download(char *fichier) {
	
	//initialisation du crc
	crcInit();
	
	//clean_stdin(); //on vide le buffer clavier
	
	/*
	 * Déclarations
	 */
	FILE* fd;
	unsigned char *buffer; //Paquet reçu.
	unsigned char *morceaufichier; //Morceau de fichier extrait de buffer.
	buffer=malloc((PACKET_SIZE+1+5)*sizeof(char));
	morceaufichier = malloc((PACKET_SIZE+1) * sizeof(char));
	fd=fopen(fichier,"ab");
	//remettre le lock (saute avec le fopen)
	setLock(fichier);
	int octet_recu=0; //Pour vérifier que la réception s'est bien faite.
	char taillefichier[100]; //Pour récupérer la taille du fichier
	//Pour la barre de progression
	int percent = 0;
	int old_percent = 0;
	//Gestion des crc
	
	char crcPaquetServeur[5];
	char crcPaquet[4];
	//taille du paquet courant (le dernier n'est pas forcement 1000)
	int current_paquet_size;
	//Pour les stats et calculs.
	int nb_pacquets_corrompus = 0;
	int nb_paquets=0;
	int k;
	
	
	
	
	//START correspond au début du transfert.
	sendCommandToClient("START"); printf("WRITE : %s\n","START");

	//On récupère la taille du fichier. Si ceci échoue, le reste est vain.
	read(tls_sock, taillefichier, sizeof(taillefichier)); if(DEBUG) printf("READ : %s\n",taillefichier);
	int taille_fichier=atoi(taillefichier);
	
	//CONTINUE permet au serveur de savoir que l'on a bien reçu le paquet, ceci avant de lancer le reste.
	sendCommandToClient("CONTINUE"); if(DEBUG) printf("WRITE : %s\n","CONTINUE");

	
	printf("Taille du fichier : %s o\n",taillefichier);
	printf("__________________________________________________\n");
	
	fseek(fd, 0, SEEK_SET);
	int taille_temporaire=ftell(fd);

	/*
	 * Lecture du md5 du fichier que l'on va recevoir.
	 */
	char bufferMD5[33]; //md5 envoyé par le serveur
	read(tls_sock, bufferMD5, 33); if(DEBUG) printf("READ : %s\n",bufferMD5);

	sendCommandToClient("CONTINUE"); if(DEBUG) printf("WRITE : %s\n","CONTINUE");
	
	time_t start, end;
	double duration = 0;
	
	time(&start);

	while(taille_temporaire<taille_fichier)
	{
		//printf("%i/%d\n",nb_paquets,(int)(((double)(taille_fichier))/((double)(PACKET_SIZE))));
		nb_paquets+=1;
		current_paquet_size=min(PACKET_SIZE, taille_fichier-taille_temporaire);


		/*
		 * Gestion de la barre de progression
		 */
		percent = (int)((50*(double)taille_temporaire)/((double)taille_fichier));
        	if(percent>old_percent)
        	{
        		
        		for(k = old_percent;k<percent;k++)
        		{
        		        printf("=");
        			fflush(stdout); //On force stdout à se rafraichir.
        		}

        		old_percent = percent;
        	}
		//do {
			//réception du paquet.
			octet_recu=read(tls_sock, morceaufichier, current_paquet_size);
		

		
		


		fwrite(morceaufichier,current_paquet_size*sizeof(char),1,fd);
		taille_temporaire+=current_paquet_size;

	}
	
	time(&end);
	for(k = percent;k<50;k++)
	{
	        printf("=");
		fflush(stdout); //On force stdout à se rafraichir.
	}
	printf("\n"); //(Juste pour terminer la progress bar)
	//fermeture du fichier
	fclose(fd);
	
	//Libération de la ram allouée au début.
	free(morceaufichier);
	free(buffer);

	//Quelques infos sur le transfert.
	printf("Corrompus : %i/%i\n",nb_pacquets_corrompus,nb_paquets);
	
	//On récupère le md5 du fichier reçu.
	char bufferT[33];
    	getMD5checkSum(fichier, bufferT); 
	
	duration = difftime(end,start);
	
	printf("Débit moyen : %f\n", ((double)taille_fichier/1000000)/duration);
	
	
	//unlock
	unLock(fichier);
	
	/*
	 * On vérifie si le fichier crée est à très grande probabilité le même que celui
	 * demandé.
	 */
	printf("Serveur : %s\nClient : %s\n",bufferMD5,bufferT);
	if(strcmp(bufferMD5,bufferT)==0)
	{
		printf("Fichier reçu correctement.\n");
		return 0; //0 => transfert réussi.
	}
	else
	{
		printf("Fichier corrompu !\n");
	}
	
	return 1; //1 => transfert échoué.
	
}


int sendCommandToClient(char* commande)
{
	char commandReader[10];
    	sprintf(commandReader, "%s", commande);
	write(tls_sock, commandReader, sizeof(commandReader));
}

/*
 * Attend le message 'commande' de la part du client, annule l'upload si reçoit autre chose.
 */
int waitingForClientMessage(char* commande)
{
	char commandReader[10];
	read(tls_sock, commandReader, sizeof(commandReader)); if(DEBUG) printf("READ : %s\n",commandReader);
	if(strcmp(commandReader,commande)!=0)
	{
		printf("Erreur protocolaire\n");
		return 1;
	}
	return 0;
}

/*
 * file_exists
 * 	filename : chemin du fichier à vérifier.
 *
 * Permet de tester l'existence d'un fichier.
 * retourne 1 si il existe, 0 sinon.
 */
int file_exists(char * filename)
{
	FILE *istream;
	if ( (istream = fopen ( filename, "r" ) ) == NULL )
	{
		return 0;
	}
	fclose ( istream );
	return 1;
}

/*
 * dir_exists
 * 	dirpath : chemin du dossier à vérifier.
 *
 * Permet de tester l'existence d'un dossier.
 * retourne 1 si il existe, 0 sinon.
 */
int dir_exists(char * dirpath)
{
	DIR  *dip;
	if ((dip = opendir(dirpath)) == NULL)
	{         
		return 0;
	}
	closedir(dip);
	return 1;
}

int getLock(char * filePath) //0 : unlock, 1 : lock ou erreur
{
	/* l_type   l_whence  l_start  l_len  l_pid   */
	struct flock fl = {F_WRLCK, SEEK_SET,   0,      0,     0 };
	int fd;
	fl.l_pid =getpid();
	if ((fd = open(filePath, O_RDWR)) == -1) {
		perror("open");
		return 1;
	}
	if (fcntl(fd, F_GETLK, &fl) == -1) {
		perror("fcntl");
		return 1;
	}
	close(fd);
	if(fl.l_type==F_UNLCK)
	{
		return 0;
	}
	
	return 1;
}



int setLock(char * filePath)// 0 : success, 1 : error
{
	if(getLock(filePath)) return 1;
	struct flock fl = {F_WRLCK, SEEK_SET,   0,      0,     0 };
	int fd;
	fl.l_pid = getpid();
	if ((fd = open(filePath, O_RDWR)) == -1) {
		perror("open");
		return 1;
	}
	if (fcntl(fd, F_SETLKW, &fl) == -1) {
		perror("fcntl");
		return 1;
	}
	return 0;
}

int unLock(char * filePath)
{
	if(!getLock(filePath)) return 1; //the file is already unlocked
	struct flock fl = {F_UNLCK, SEEK_SET,   0,      0,     0 };
	int fd;
	if ((fd = open(filePath, O_RDWR)) == -1) {
		perror("open");
		return 1;
	}
	fl.l_pid =getpid();
	if (fcntl(fd, F_SETLK, &fl) == -1) {
		perror("fcntl");
		return 1;
	}
    	return 0;
}




int getLocked(char * filePath) //0 : unlock, sinon : lock
{
	int k = 0;
	while(strcmp(lockedFiles[k],"\r")!=0 && strcmp(lockedFiles[k],filePath)!=0)
	{
		k++;
	}
	return strcmp(lockedFiles[k],"\r");
}



int setLocked(char * filePath)// 0 : success, 1 : error
{
	int k = 0;
	while(strcmp(lockedFiles[k],"\r")!=0 && strcmp(lockedFiles[k],"\0")!=0)
	{
		k++;
	}
	
	lockedFiles[k] = filePath;
	if(strcmp(lockedFiles[k],"\r")!=0)
	{
		lockedFiles[k+1] = "\r";
	}
	
	return 1;
}

int unLocked(char * filePath)
{
	printf("1\n");
	int k = 0;
	while(strcmp(lockedFiles[k],"\r")!=0 && strcmp(lockedFiles[k],filePath)!=0)
	{
		k++;
	}
	printf("1\n");
	lockedFiles[k]="\0";
	printf("1\n");
	/*if(strcmp(lockedFiles[k+1],"\r")!=0)
	{
		printf("2\n");
		lockedFiles[k] = "\r";
		printf("2\n");
	}*/
	printf("1\n");
	return 1;
}
