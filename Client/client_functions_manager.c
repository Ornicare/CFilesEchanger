#include <stdio.h>
#include <stdlib.h>
#include <linux/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
//pour le md5
#include <openssl/md5.h>
#include "md5manager.h"
#include <time.h>
#include <fcntl.h>

#include "crc.h"

#define PACKET_SIZE 1 //pas plus, ça passe pas
#define DEBUG 1



void clean_stdin(void)
{
    int c;
 
    do {
        c = getchar();
    } while (c != '\n' && c != EOF);
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

int sendCommandToServer(int sock, char* commande)
{
	char commandReader[10];
    	sprintf(commandReader, "%s", commande);
	write(sock, commandReader, sizeof(commandReader));
}

/*
 * download : 
 * 	fichier : chemin du fichier à créer.
 *	socket : socket serveur
 * Permet de télécharger un fichier envoyé par le serveur.
 */

int download(char *fichier,int socket) {
	
	//initialisation du crc
	crcInit();
	
	
	
	

	
	
	
	
	
	/*
	 * On vérifie que le fichier n'exite pas déjà chez le client.
	 */
	if(file_exists(fichier)==1) {
		
		printf("&&&&&&&&%i         %i\n",getLock(fichier), getpid());
		//On vérifie que le fichier n'est pas locké par une autre application et qu'il existe
		if(getLock(fichier))
		{
			//sendCommandToServer(socket, "ABORT"); printf("WRITE : %s\n","ABORT");
			printf("File locked\n");
			return 2;
		}
		printf("&&&&&&&&%i         %i\n",setLock(fichier), getpid());
		
		printf("The file %s is already existing. Do you want to override it ? (O/n)\n", fichier);
		char response[10];
		//le %9s fera que scan ne lira que les 9 premiers char tapés. (pas de buffer overflow).
		if ( scanf ( "%9s", response ) != 1 ) 
 			return 1; //2 => transfert échoué (ici le scanf).
		if(strcmp(response,"O")!=0 && strcmp(response,"o")!=0)
		{
			clean_stdin(); //on vide le buffer clavier
			return 2; //2 => transfert annulé.
		}
		remove(fichier);

	}
	
	
	
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
	sendCommandToServer(socket, "START"); printf("WRITE : %s\n","START");

	//On récupère la taille du fichier. Si ceci échoue, le reste est vain.
	read(socket, taillefichier, sizeof(taillefichier)); if(DEBUG) printf("READ : %s\n",taillefichier);
	int taille_fichier=atoi(taillefichier);
	
	//CONTINUE permet au serveur de savoir que l'on a bien reçu le paquet, ceci avant de lancer le reste.
	sendCommandToServer(socket, "CONTINUE"); if(DEBUG) printf("WRITE : %s\n","CONTINUE");

	
	printf("Taille du fichier : %s o\n",taillefichier);
	printf("__________________________________________________\n");
	
	fseek(fd, 0, SEEK_SET);
	int taille_temporaire=ftell(fd);

	/*
	 * Lecture du md5 du fichier que l'on va recevoir.
	 */
	char bufferMD5[33]; //md5 envoyé par le serveur
	read(socket, bufferMD5, 33); if(DEBUG) printf("READ : %s\n",bufferMD5);

	sendCommandToServer(socket, "CONTINUE"); if(DEBUG) printf("WRITE : %s\n","CONTINUE");
	
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
			octet_recu=read(socket, morceaufichier, current_paquet_size);
		

		
		


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

/*
 * La fonction min.
 */
int min(int a, int b)
{
	int tmp = a;
	if(a>b) tmp=b;
	return tmp;
}


char * toLowerCase(char * word)
{
	int i;
	for (i = 0; word[i]; i++)
	word[i] = tolower(word[i]);
	return word;
}

	
int parseCommand(int sock, char* mesg)
{	

	char buffer[256];
	strncpy(buffer,mesg,strlen(mesg));
	buffer[strlen(mesg)]='\0';
	/*
	 * Préformatage des chaines.
	 */
	int k=0;
	while(buffer[k]!=' ')
	{
		k++;
	}
	buffer[k]='\0';
	k++;
	
	if(strcmp(buffer,"GET")==0)
	{
		char argument[256];
		int temp_k = k;
		while(buffer[k]!='\0' && k<255)
		{
			argument[k-temp_k]=buffer[k];
			k++;
		}
		argument[k-temp_k]='\0';
		
		
		char fileAvailable[256];
		read(sock, fileAvailable, sizeof(fileAvailable)); printf("READERR : %s\n",fileAvailable);
		//printf("@%s\n",fileAvailable);
		if(strcmp(fileAvailable,"FILE_EXIST\0"))
		{
			write(sock, "CONTINUE\0",9); if(DEBUG) printf("WRITE : %s\n","CONTINUE\0");
			//printf("2%s\n",fileAvailable);
			//read(sock, buffer, sizeof(buffer));
			//printf("3%s\n",fileAvailable);
			read(sock, buffer, sizeof(buffer)); if(DEBUG) printf("READ : %s\n",buffer);
			return 3;
		}
		if(download(argument,sock)==2)
		{
			write(sock, "CANCEL\0",7); if(DEBUG) printf("WRITE : %s\n","CANCEL\0"); //Attention au paquets fantomes... envoyer 11 dans 9...
		}
		else
		{
			printf("Téléchargement terminé.\n");
		}
		//write(sock,"OK\0",3);
	}
	
	
	memset( buffer, '\0', sizeof(buffer)); //flush the buffer
	read(sock, buffer, sizeof(buffer)); if(DEBUG) printf("READ2 : %s\n",buffer);
	while(strcmp(buffer,"ENDSTREAM")!=0 && strcmp(buffer,"")!=0)
	{
		//buffer[0]='A';
		write(sock, "CONTINUE\0",9); if(DEBUG) printf("WRITE : %s\n","CONTINUE\0");
		read(sock, buffer, sizeof(buffer)); if(DEBUG) printf("READ : %s\n",buffer);
		//printf("%s\n",buffer);
	}
	
	
	//
	char temp[256];
	//read(sock,temp,sizeof(temp)); printf("READ : %s\n",temp);

	return 0;
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
	struct flock fl = {F_RDLCK, SEEK_SET,   0,      0,     0 };
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


