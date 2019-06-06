#include "pse.h"

#define BUF_SIZE 140
#define CMD "serveur"
#define MaxUser 10


typedef struct Thread {
	pthread_t Rid;
	pthread_t Wid;
	int tid;
	int canal;
	sem_t sem;
	sem_t sem_w;
	sem_t sem_r;
} Thread;

struct sockaddr_in adrEcoute, adrClient;
Thread thread[MaxUser];
sem_t sem_;

unsigned int lgAdrClient;
int ecoute, ret;


void *ReceiveClient(void *arg)
{
	Thread *tc = (Thread*) arg;
	char buf[BUF_SIZE];
	int nbRead;
	int continu =1;
	sem_wait(&tc->sem);
	sem_wait(&tc->sem_r);
	nbRead = lireLigne(tc->canal,buf);
	sem_post(&tc->sem_w);
	while (continu);
	
	exit(EXIT_SUCCESS);
}

void *SendClient(void *arg)
{
	Thread *tc = (Thread*) arg;
	char buf[BUF_SIZE];
	int nbRead;
	int continu =1;
	sem_wait(&tc->sem);
	ecrireLigne(tc->canal,"Rentrez votre pseudo : ");
	sem_post(&tc->sem_r);
	sem_wait(&tc->sem_w);
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
	
	while (continu);
	
	exit(EXIT_SUCCESS);
}


int main(int argc, char *argv[])
{
	short port;
	
	if (argc != 2)
		erreur("Mauvaise syntaxe : %s port\n", argv[0]);
	
	port = (short) atoi(argv[1]);
	sem_init( &sem_, 0, MaxUser);
	for (int i = 0; i < MaxUser; i++)
	{
		thread[i].tid = i;
		thread[i].canal = -1;
		sem_init(&thread[i].sem, 0, 0);
		sem_init(&thread[i].sem_w, 0, 0);
		sem_init(&thread[i].sem_r, 0, 0);
		if (pthread_create(&thread[i].Rid, NULL, ReceiveClient, NULL) != 0) 
			erreur_IO("creation of the reading thread");
		printf("Lancement du thread de lecture n°%d | ",thread[i].tid);
		if (pthread_create(&thread[i].Wid, NULL, SendClient, NULL) != 0)
			erreur_IO("creation of the writing thread");
		printf("Lancement du thread d'écriture n°%d\n", thread[i].tid);
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
			if ( thread[i].canal == -1)
			{
				libre = i;
				trouve = 1;
			}
			i++;
		}
		thread[libre].canal = accept(ecoute, (struct sockaddr *) &adrClient, &lgAdrClient);
		if (thread[libre].canal < 0)
			erreur_IO("accept");
		
		printf("%s : Connexion sur le socket depuis le thread %d\n", CMD, libre);
		printf("%s : Adresse %s, port %hu\n", CMD, stringIP(ntohl(adrClient.sin_addr.s_addr)), ntohs(adrClient.sin_port));
		sem_post(&thread[libre].sem);
	}
	exit(EXIT_SUCCESS);
}
