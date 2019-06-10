#include "pse.h"
#include "data_function.h"


#define BUF_SIZE 140
#define CMD "serveur"
#define MaxUser 10



enum states {NOT_CONNECTED, CONNECTED};

enum events {SEND_CONNECTION_DEMAND, SEND_CONNECTION_SUCCESS, PUBLISH, LIKE, FOLLOW, UNFOLLOW};

typedef struct {
	pthread_t Rid;
	pthread_t Wid;
	int tid;
	int canal;
	sem_t sem;
	sem_t sem_w;
	sem_t sem_r;
	enum states state;
	enum states event;
} Thread_Data;

struct sockaddr_in adrEcoute, adrClient;
Thread_Data thread_data[MaxUser];
sem_t sem_;

unsigned int lgAdrClient;
int ecoute, ret;

DataUtilisateur users;
DataInfo info;

void *receiveClient(void *arg)
{
	Thread_Data *tc = (Thread_Data*) arg;
	DataUtilisateur *courant;
	char buf[BUF_SIZE];
	int nbRead;
	int continu = 1;
	while (continu)
	{
		sem_wait(&tc->sem_r);
		printf("On arrive ici\n");
		switch(tc->state)
		{
			case (NOT_CONNECTED) :
				nbRead = lireLigne(tc->canal,buf);
				courant = findUserByPseudo(&users, buf);
				if(courant != NULL)
				{
					printf("%s se connecte au serveur\n", courant->utilisateur->pseudo);
					tc->state = CONNECTED;	//À virer après
					tc->event = SEND_CONNECTION_SUCCESS;
				}
				else
				{
					//L'utilisateur n'existe pas
					printf("erreur ... %s inconnu\n", buf);
				}
				sem_post(&tc->sem_w);
				break;
			case (CONNECTED) :
				break;
		}
	}
	
	exit(EXIT_SUCCESS);
}

void *sendClient(void *arg)
{
	Thread_Data *tc = (Thread_Data*) arg;
	char buf[BUF_SIZE];
	int nbRead;
	int continu =1;
	sem_wait(&tc->sem_w);
	while (continu)
	{
		switch(tc->event)
		{
			case (SEND_CONNECTION_DEMAND) :
				strcpy(buf, "Rentrez votre pseudo");
				break;
			case (SEND_CONNECTION_SUCCESS) : 
				strcpy(buf, "Bienvenue vous êtes connecté");
				break;
		}
		printf("On envoie un message %d\n", tc->canal);
		ecrireLigne(tc->canal, buf);
		sem_post(&tc->sem_r);
		sem_wait(&tc->sem_w);
	}
	/*
	if (existe in liste_pseudo)
	{
		ecrireLigne(tc->canal, "\nTapez votre mot de passe : ");
		sem_post(&tc->sem_r);
	}
	else
	{
		ecrireLigne(tc->canal, "Aucun mot de passe n'est associé à ce pseudo, veuillez en choisir un : ");
	}
	*/
	
	
	exit(EXIT_SUCCESS);
}


int main(int argc, char *argv[])
{
	//chargeement de la base de données
	info.nb_utilisateur = 0;
	initDataUtilisateur(&users);
	loadDataFromFile(&users, &info);
	printData(&users);
	printf("test 3\n");
	short port;
	
	if (argc != 2)
		erreur("Mauvaise syntaxe : %s port\n", argv[0]);
	
	port = (short) atoi(argv[1]);
	sem_init( &sem_, 0, MaxUser);
	for (int i = 0; i < MaxUser; i++)
	{
		thread_data[i].tid = i;
		thread_data[i].state = NOT_CONNECTED;
		thread_data[i].event = NOT_CONNECTED;
		thread_data[i].canal = -1;
		sem_init(&thread_data[i].sem, 0, 0);
		sem_init(&thread_data[i].sem_w, 0, 0);
		sem_init(&thread_data[i].sem_r, 0, 0);
		if (pthread_create(&thread_data[i].Rid, NULL, receiveClient, &thread_data[i]) != 0) 
			erreur_IO("creation of the reading thread");
		printf("Lancement du thread de lecture n°%d | ",thread_data[i].tid);
		if (pthread_create(&thread_data[i].Wid, NULL, sendClient, &thread_data[i]) != 0)
			erreur_IO("creation of the writing thread");
		printf("Lancement du thread d'écriture n°%d\n", thread_data[i].tid);
	}
	
	printf("%s : Creation du socket\n", CMD);
	ecoute = socket(AF_INET, SOCK_STREAM, 0);
	if (ecoute < 0)
		erreur_IO("socket");
	
	adrEcoute.sin_family = AF_INET;
    adrEcoute.sin_addr.s_addr = INADDR_ANY;
    adrEcoute.sin_port = htons(port);
    printf("%s : Liaison de l'adresse serveur sur le port %d\n", CMD, port);
    ret = bind( ecoute, (struct sockaddr *) &adrEcoute, sizeof(adrEcoute));
    if (ret < 0)
    	erreur_IO("liaison");
    
    printf("%s : En attente de connexion sur le socket\n", CMD);
    ret = listen(ecoute, 5);
	if (ret < 0)
		erreur_IO("ecoute");
	
	while (1)
	{
		int libre = -1;
		int trouve = 0;
		int i = 0;
		sem_wait(&sem_);
		while (trouve == 0)
		{
			if ( thread_data[i].canal == -1)
			{
				libre = i;
				trouve = 1;
			}
			i++;
		}
		thread_data[libre].canal = accept(ecoute, (struct sockaddr *) &adrClient, &lgAdrClient);
		if (thread_data[libre].canal < 0)
			erreur_IO("accept");
		
		printf("%s : Connexion sur le socket depuis le thread %d\n", CMD, libre);
		printf("%s : Adresse %s, port %hu\n", CMD, stringIP(ntohl(adrClient.sin_addr.s_addr)), ntohs(adrClient.sin_port));
		sem_post(&thread_data[libre].sem_w);
		printf("Lancement de la discussion client/serveur\n");
	}
	exit(EXIT_SUCCESS);
}

