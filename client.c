#include "pse.h"
#define BUF_SIZE 1024
#define CMD "client"

typedef struct Thread {
	pthread_t Rid;
	pthread_t Wid;
	int tid;
	int canal;
	sem_t sem;
	sem_t sem_w;
	sem_t sem_r;
	int fin;
} Thread_Data;


void *listenServer(void *arg)
{
	Thread_Data *tc = (Thread_Data*) arg;
	char buf[BUF_SIZE];
	int nbRead = 0;
	while (1)
	{
		nbRead = lireLigne(tc->canal,buf);
		if(nbRead == -1)
			erreur_IO("lireLigne");
		printf("Reception: %s", buf);
		//On a reçu un message on laisse le client répondre
		sem_post(&tc->sem_w);
	}
	
	exit(EXIT_SUCCESS);
}

void *sendServer(void *arg)
{
	Thread_Data *tc = (Thread_Data*) arg;
	char buf[BUF_SIZE];
	int nbWrite = 0;
	//Discussion entre le client et le serveur
	
	while(strcmp(buf, "fin\n") != 0)
	{
		sem_wait(&tc->sem_w);
		printf(">");
		if(fgets(buf, LIGNE_MAX, stdin) == NULL)
			erreur("fegts null\n");
		nbWrite = ecrireLigne(tc->canal, buf);
		if(nbWrite == -1)
			erreur_IO("ecrireLigne");
		printf("%s: Commande envoyé au serveur\n", CMD);
	}
	

	
	exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{	
	int canal, ret;
	struct sockaddr_in* adrServ;
	Thread_Data thread_data;
	
	if(argc != 3)
		erreur("usage: %s machine port\n", argv[0]);
	
	printf("%s: Création du socket\n", CMD);
	canal = socket(AF_INET, SOCK_STREAM, 0);
	if(canal < 0)
		erreur_IO("socket");
	
	printf("%s: Résolution DNS pour %s sur le port %s\n", CMD, argv[1], argv[2]);
	adrServ = resolv(argv[1], argv[2]);
	if(adrServ == NULL)
		erreur("Adresse %s port %s inconnue\n", argv[1], argv[2]);

	printf("%s : Adresse %s, port %hu\n", CMD, stringIP(ntohl(adrServ->sin_addr.s_addr)), ntohs(adrServ->sin_port));
	
	printf("%s: Connection au socket\n", CMD);
	ret = connect(canal, (struct sockaddr *) adrServ, sizeof(struct sockaddr_in));
	if(ret<0)
		erreur_IO("connect");
		
	thread_data.tid = 1;
	thread_data.canal = canal;
	sem_init(&thread_data.sem, 0, 0);
	sem_init(&thread_data.sem_w, 0, 0);
	sem_init(&thread_data.sem_r, 0, 0);
	if (pthread_create(&thread_data.Rid, NULL, listenServer, &thread_data) != 0) 
		erreur_IO("creation of the reading thread");
	printf("Lancement du thread de lecture n°%d | ",thread_data.tid);
	if (pthread_create(&thread_data.Wid, NULL, sendServer, &thread_data) != 0)
		erreur_IO("creation of the writing thread");
	printf("Lancement du thread d'écriture n°%d\n", thread_data.tid);
	
	//attente de la fin des thread
	pthread_join(thread_data.Wid, NULL);
		
	if (close(canal) == -1)
		erreur_IO("cloe");
		
	return 0;
}
