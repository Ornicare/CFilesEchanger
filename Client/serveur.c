#include <stdio.h>
#include <stdlib.h>
#include <linux/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <pthread.h>
//pour list()
#include <sys/types.h>
#include <dirent.h>
//pour pwd()
#include <unistd.h>
#include <errno.h>

#include "serveur_functions_manager.h"

#define TAILLE_MAX_NOM 256
#define PACKET_SIZE 1000


//Attention : compilation : gcc serveurTh.c -lpthread -o serveur.exe

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct hostent hostent;
typedef struct servent servent;

/**************************************/
//int static socket_descriptor;


/**************************************/



/**************************************/
//Variables globales.



int main(int argc, char** argv)
{
	int port; //Le port qui va être utilisé.
	int nouv_socket_descriptor,
	longeur_adresse_courante;
	sockaddr_in adresse_locale,
	adresse_client_courant;
	hostent* ptr_hote;
	//servent* ptr_service;
	char machine[TAILLE_MAX_NOM+1]; //what is it ?

	//On vérifie que l'utilisateur à bien fournit le port... on ne sait jamais avec eux.
	if(argc != 2) {
		perror("usage : serveur <port>");
		exit(1);
	}

	port = atoi(argv[1]); //int atol(char*) /* Assez utile :) */

	gethostname(machine,TAILLE_MAX_NOM);

	if((ptr_hote = gethostbyname(machine)) == NULL)
	{
		perror("erreur : impossible de trouver le serveur a partir de son nom.");
		exit(1);
	}
	printf("-----------------------------------------------------\n");
	printf("Début de la création du serveur...");
	bcopy((char*)ptr_hote->h_addr, (char*)&adresse_locale.sin_addr, ptr_hote->h_length);
	adresse_locale.sin_family = ptr_hote->h_addrtype;
	adresse_locale.sin_addr.s_addr = INADDR_ANY;
	printf("DONE\n");
	adresse_locale.sin_port = htons(port);

	printf("Le port %d va lui être attribué.\n", ntohs(adresse_locale.sin_port));


	printf("Création de la socket...");

	if((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("erreur : impossible de creer la socket");
		exit(1);
	}
	printf("DONE\n");
	printf("Bindage du port...");
	if((bind(socket_descriptor, (sockaddr*)(&adresse_locale), sizeof(adresse_locale))) < 0){
		perror("erreur : le port n'a pas pu être attribué.");
		exit(1);
	}
	printf("DONE\n");

	printf("Création du serveur terminé.\n");
	printf("-----------------------------------------------------\n");
	printf("En attente de clients...\n");
	listen(socket_descriptor,5); //5 clients peuvent se connecter en même temps (au maximum).

	//Attente des connexions (infinite loop)
	pthread_t thread;
	for(;;){

		longeur_adresse_courante = sizeof(adresse_client_courant);
		if((nouv_socket_descriptor = accept(socket_descriptor,(sockaddr*)(&adresse_client_courant),&longeur_adresse_courante)) <0){ //fonction bloquante : accept
			perror("erreur : impossible d'accepter la connexion avec le client.");
			exit(1);
		}

		printf("[%i]Reception d'un message.\n",nouv_socket_descriptor);
		if(pthread_create( &thread, NULL, talker, (void *)nouv_socket_descriptor ) < 0) {
			perror("erreur : le thread n'a pas pu être crée");
			exit(1);
		}
	}

	return 0;
}
