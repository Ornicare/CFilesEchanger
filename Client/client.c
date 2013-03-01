/*---------------------------------------------------
Client a lancer apres le serveur avec la commande : 
client <adresse-serveur> <message-a-transmettre>
-----------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <linux/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
//pour le md5
#include <openssl/md5.h>
#include "md5manager.h"
#include "client_functions_manager.h"
#define DEBUG 1

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct hostent hostent;
typedef struct servent servent;

int main(int argc, char** argv) 
{

	
	int longueur; /* longueur d'un buffer utilisé */
	sockaddr_in adresse_locale; /* adresse de socket locale */
	hostent *ptr_host; /* info sur une machine hôte */
	//servent *ptr_service; /*info sur le service */
	char buffer[256];
	char *prog; /* nom du programme */
	char *host; /* nom de la machine distante */
	char *mesg; /* message envoye */
	int port;
	
	
	if(argc != 4) {
		perror("usage : client <adresse-serveur> <message-a-transmettre> <port>");
		exit(1);
	}
	
	printf("Récupération des arguments\n");
	prog = argv[0];
	printf("Nom de l'executable : %s \n", prog);
	host = argv[1];
	printf("Adresse du serveur: %s \n", host);
	mesg = argv[2];
	printf("Message à envoyer: %s \n", mesg);
	port = atoi(argv[3]);
	printf("Port utilisé : %i \n", port);
	
	
	
	if ((ptr_host = gethostbyname(host)) == NULL) {
		perror("erreur : impossible de trouver le serveur a partir de son adresse");
		exit(1);
	}
	
	/* copie caractere par caractere des infos de ptr_host vers adresse_locale */
	bcopy((char*)ptr_host->h_addr, (char*)&adresse_locale.sin_addr, ptr_host->h_length);
	adresse_locale.sin_family = AF_INET; /* ou ptr_host->h_addrtype */
	
	adresse_locale.sin_port = htons(port); //le port utilise
	
	printf("Numero de port pour la connexion au serveur : %d \n", ntohs(adresse_locale.sin_port));
	
	/* creation de la socket */
	if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("erreur : impossible de creeer la connexion avec le serveur");
		exit(1);
	}

	if((connect(socket_descriptor, (sockaddr*)(&adresse_locale), sizeof(adresse_locale))) < 0){
		perror("impossible de lier la socket avec client");
		exit(1);
	}
	
	printf("connexion etablie avec le serveur \n");
	
	printf("envoi d'un message au serveur \n");

	printf("Notice : commands are all in UPPERCASE. type \"HELP\" to get some help\n");


	/*
	 * Entrée dans le module d'envoi de messages
	 */
	
	char message[256];
	while(strcmp(message,"STOP")!=0)
	{
		memset( message, '\0', sizeof(message));
		printf("Entrez votre commande\n");
		//if ( scanf ( "%255s", message ) == 1 ) //scanf arrête la lecture au premier char d'espacement........
		if(fgets(message, sizeof(message), stdin)!=NULL)
 		{	
 			message[strlen(message)-1]='\0'; //Pour remplacer le \r par un \0
 			
 			
 			
 			//printf("%i @%s@\n",strcmp(message,"STOP"),message);
 			
 			/*char * lf = strchr(message, '\n'); // On cherche le caractere '\n'. 
        		if (lf != NULL) // S'il est present, ... 
            			*lf = '\0'; // ... on le supprime    */
 			//printf("@%s@",message);
 			
 			if ((write(socket_descriptor, message, sizeof(message))) < 0) {
				perror("erreur : impossible d'ecrire le messsage destine au serveur \n");
				exit(1);
			}
			else
			{
			
				if(DEBUG) printf("WRITE : %s\n",message);
				
				read(socket_descriptor, buffer, sizeof(buffer));  if(DEBUG) printf("READ : %s\n",buffer);
				write(socket_descriptor,"RECEIVED\0",9);  if(DEBUG) printf("WRITE : %s\n","RECEIVED\0");
			}
			parseCommand(socket_descriptor, message);
 		}
		
	}
		
	/* envoi du dit message */
	/*if ((write(socket_descriptor, mesg, strlen(mesg))) < 0) {
		perror("erreur : impossible d'ecrire le messsage destine au serveur \n");
		exit(1);
	}
	
	
	
	/* mise en attente du programme pour simuler un delai de transmission */
	//sleep(3);
	
	//printf("Message envoye au serveur \n");
	
	/* lecture de la reponse en provenance du serveur */
	/*while((longueur = read(socket_descriptor, buffer, sizeof(buffer))) > 0) { //A améliorer, bloquant.
		//printf("Reponse du serveur : ");
		write(1, buffer, longueur);
	}*/
	
	printf("\nFin de la reception.\n");
	
	close(socket_descriptor);
	
	printf("Connexion avec le serveur fermee, fin du pgrm.\n");
	
	exit(0);
}
