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


	

	char buffer[256];
	char temp[256];
	//
	int longueur;
	int sock;
	
	
	
	char asciiCode[2]="1";

	sock = (int)d; //bah, faut bien récupérer l'id du socket... on cast void* en int...
	
	currentPaths[sock] = pgetcwd(); //initialisation du chemin courant.

	while(1)
	{
		printf("[%i]Attente d'un message.\n",sock);
		//do
		//{
			longueur = read(sock, buffer, sizeof(buffer)); printf("READ : @%s@%d\n",buffer,buffer[0]);
			//sprintf(asciiCode,"%d",buffer[0]);
		//} while(strcmp(asciiCode,"0")==0);
		//buffer[longueur] = '\0';
		//if(strcmp(asciiCode,"0")!=0)
		//{
			if(longueur <= 0) return;  //si la lecture échoue, on annule tout.
		
			write(sock, "RECEIVED\0",9);  if(DEBUG) printf("WRITE : %s\n","RECEIVED\0");
		
			//
			read(sock, temp, sizeof(temp)); if(DEBUG) printf("READ : %s\n",temp);
		
		
		
			if(parseCommand(buffer, sizeof(buffer), sock))
			{
				printf("[%i]Une erreur est survenue, fermeture du socket.\n",sock);	
				break;
			}
			else
			{
				//printf("%s\n", buffer);
				//printf("[%i]Commande réussie : %s\n",sock,buffer);
				write(sock, "ENDSTREAM\0", 10); if(DEBUG) printf("WRITE : %s\n","ENDSTREAM\0");
			}
		//}
	}

	close(sock); //on ferme le socket.
}

/*
 * Fonction d'analyse du message envoyé par le client.
 */
int parseCommand(char* BUFFER, int BUFFER_LENGTH, int sock) { //     /!\utiliser fork ou mutex
	
	/*
	 * Préformatage des chaines.
	 */
	int k=0;
	while(BUFFER[k]!=' ')
	{
		k++;
	}
	BUFFER[k]='\0';
	k++;
	
	
	
	if(strcmp(BUFFER,"LIST")==0) //Liste le répertoire courant.
	{
		list(sock);
	}
	else if(strcmp(BUFFER,"PWD")==0) //récupère le répertoire courant.
	{
		pwd(sock);
	}
	else if(strcmp(BUFFER,"STOP")==0) //si le message envoyé est "STOP", on quitte tout.
	{
		stop(sock);
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
		uploadManager(sock, argument);
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
		cdDown(sock, argument);
	}
	else if(strcmp(BUFFER,"CDUP")==0) 
	{
		cdUp(sock);
	}
	return 0;
}

int cdDown(int sock,char* argument)
{
	char testPath[255];
	strncpy(testPath,currentPaths[sock],strlen(currentPaths[sock]));
	testPath[strlen(currentPaths[sock])]='\0';
	
	//Rajouter un / à la fin s'il n'y est pas
	if(argument[strlen(argument)-1]!='/')
	{
		strcat(argument,"/\0");	
	}
	
	//sprintf(testPath, "%s%s", currentPath, argument); 
	//strcat(testPath,currentPath);
	strcat(testPath,argument);
	printf("DEBUG : %s\n",testPath);
	if(dir_exists(testPath)) strncpy(currentPaths[sock],testPath,strlen(testPath));
	
	write(sock,currentPaths[sock],strlen(currentPaths[sock]));
	if(waitingForClientMessage(sock,"CONTINUE")) return 1;
	
	//chdir(currentPath);
	return 0;
}

int cdUp(int sock)
{
	if(strcmp(currentPaths[sock],"/")==0) return 0; //on est déjà à la racine.
	char buffer[strlen(currentPaths[sock])+1];
	strncpy(buffer, currentPaths[sock], strlen(currentPaths[sock]));
	int k=strlen(currentPaths[sock])-1;
	do
	{
		buffer[k]='\0';
		k--;
	} while(buffer[k]!='/');
	buffer[strlen(currentPaths[sock])]='\0';
	strncpy(currentPaths[sock],buffer,sizeof(buffer));
	write(sock,buffer,sizeof(buffer));
	if(waitingForClientMessage(sock,"CONTINUE")) return 1;
	
	printf("@%s@\n",currentPaths[sock]);
	//chdir(currentPath); //choisit le dossier courant.
	return 0;
}

/*
 * Liste les fichiers et dossiers du répertoire courant (chdir()=>choisir ce répertoire)
 */
void list(int sock) {
	struct dirent *lecture;
	DIR *rep;
	printf("qdsqdsq\n");
	rep = opendir(currentPaths[sock]);
	printf("%s",currentPaths[sock]);
	write(sock,"\n**Dossiers : \n\0",16);
	waitingForClientMessage(sock,"CONTINUE");
	printf("**Dossiers : \n");
	listByType(sock,DT_DIR,rep);
	
	rep = opendir(currentPaths[sock] );
	write(sock,"\n**Fichiers : \n\0",16);
	waitingForClientMessage(sock,"CONTINUE");
	printf("**Fichiers : \n");
	listByType(sock,DT_REG,rep);
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
void listByType(int sock, int type, DIR* rep) {
	char buffer[256];
	struct dirent *lecture;
	while ((lecture = readdir(rep))) {
		if(strcmp(lecture->d_name, ".") && strcmp(lecture->d_name, ".."))
	      	{
			if(lecture->d_type == type)
		 	{
		 		strncpy(buffer,lecture->d_name,strlen(lecture->d_name));
		 		buffer[strlen(lecture->d_name)]='\0';
		    		write(sock,buffer,strlen(lecture->d_name)+1);
				printf("%s\n", lecture->d_name, lecture->d_type);
				waitingForClientMessage(sock,"CONTINUE");
		 	}
	      	}
	}
	closedir(rep);
}

/*
 * Renvoie le répertoire courant à l'utilisateur.
 */
void pwd(int sock) {
	write(sock, currentPaths[sock], strlen(currentPaths[sock]));
	waitingForClientMessage(sock,"CONTINUE");
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
void stop(int sock) {
	printf("[%i]Fermeture du serveur.\n",sock);
	char* message = "Fermeture du serveur...\n";
	write(sock,message,strlen(message)); printf("WRITE : %s\n",message);
	close(sock);
	close(socket_descriptor);
	exit(1);
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
int uploadManager(int sock, char* argument)
{
	char filePath[1024];
	strncpy(filePath,currentPaths[sock],strlen(currentPaths[sock]));
	strcat(filePath,argument);
	if(!file_exists(filePath))
	{	
		write(sock,"ERR:FILE_NOT_EXISTS\0",20);  if(DEBUG) printf("WRITE : %s : %s\n","ERR:FILE_NOT_EXISTS\0",filePath);
		char temp[256];
		read(sock,temp,sizeof(temp)); if(DEBUG) printf("READ : %s\n","temp");
		return 1;
	}
	
	write(sock,"FILE_EXIST\0",11);  if(DEBUG) printf("WRITE : %s\n","FILE_EXIST\0");
	
	//char temp[256];
	//read(sock,temp,sizeof(temp));
	
	printf("Début de l'upload\n");
	if(upload(filePath, sock)) return 1;
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
int upload(char* fichier, int socket)
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
	if(waitingForClientMessage(socket,"START")) return 1;
	
	//envoi de la taille du fichier au client
	write(socket, tailleFichier, sizeof(tailleFichier)); if(DEBUG) printf("WRITE : %s\n",tailleFichier);

	//réception de CONTINUE.
	if(waitingForClientMessage(socket,"CONTINUE")) return 1;
	//envoi du md5 du fichier
	
	getMD5checkSum(fichier, bufferT); //md5 du fichier envoyé.
	write(socket, bufferT, sizeof(bufferT));  if(DEBUG) printf("WRITE : %s\n",bufferT);

	if(waitingForClientMessage(socket,"CONTINUE")) return 1;
	

	printf("__________________________________________________\n");
	
	while  (taille_lue<taille_fichier)
	{
		nb_paquets+=1; //On compte le nombre de paquets
		current_paquet_size=min(PACKET_SIZE,taille_fichier-taille_lue); //la taille du paquet courant
		fread(morceaufichier,PACKET_SIZE,1,fs); //lecture du bloc de fichier.
		
		 
		morceaufichier[current_paquet_size]='\0'; //au cas où...
		//buffer[4]='\0';
		//printf("%s\n",buffer);
		//calcul du crc du morceau de fichier.	
		sprintf(buffer, "%X", crcFast(morceaufichier, min(PACKET_SIZE,taille_fichier-taille_lue))); //mettre le crc au début du paquet.
		
		//On met le moreceau de fichier dans le buffer.
		for(k = 0;k<(PACKET_SIZE);k++)
		{
			buffer[4+k]=morceaufichier[k];
		}
		buffer[PACKET_SIZE+5]='\0';
		
		
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
			
			octet_envoye=write(socket, buffer, min(PACKET_SIZE+1+5,taille_fichier-taille_lue+6));
			
			//code de test
			/*buffer[old_char_index]=old_char;*/
			
			//lecture de la réponse du client.
			//octet_envoye=read(socket, commandReader, sizeof(commandReader));
		//} while (strcmp(commandReader,"CONTINUE")!=0 || octet_envoye<0); //Si le client n'a pas reçu ou si y'a une erreur d'envoi
		
		/*
		 * Gestion de la progress bar.
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

	return 0;
}

/*
 * Attend le message 'commande' de la part du client, annule l'upload si reçoit autre chose.
 */
int waitingForClientMessage(int socket,char* commande)
{
	char commandReader[10];
	read(socket, commandReader, sizeof(commandReader)); if(DEBUG) printf("READ : %s\n",commandReader);
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



