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
} Thread_Data;


void *listenServer(void *arg)
{
	Thread_Data *tc = (Thread_Data*) arg;
	char buf[BUF_SIZE];
	int nbRead;
	int continu =1;
	
	while (continu);
	
	exit(EXIT_SUCCESS);
}

void *sendServer(void *arg)
{
	Thread_Data *tc = (Thread_Data*) arg;
	char buf[BUF_SIZE];
	int nbRead;
	int continu = 1;
	
	while (continu);
	
	exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{	
	int soc, ret;
	struct sockaddr_in* adrServ;
	char ligne[BUF_SIZE];
	int lgEcr;
	int nbWrite = 0;
	int nbRead = 0;
	
	if(argc != 3)
		erreur("usage: %s machine port\n", argv[0]);
	
	printf("%s: Création du socket\n", CMD);
	soc = socket(AF_INET, SOCK_STREAM, 0);
	if(soc < 0)
		erreur_IO("socket");
	
	printf("%s: Résolution DNS pour %s sur le port %s\n", CMD, argv[1], argv[2]);
	adrServ = resolv(argv[1], argv[2]);
	if(adrServ == NULL)
		erreur("Adresse %s port %s inconnue\n", argv[1], argv[2]);

	printf("%s : Adresse %s, port %hu\n", CMD, stringIP(ntohl(adrServ->sin_addr.s_addr)), ntohs(adrServ->sin_port));
	
	printf("%s: Connection au socket\n", CMD);
	ret = connect(soc, (struct sockaddr *) adrServ, sizeof(struct sockaddr_in));
	if(ret<0)
		erreur_IO("connect");
		
	//Discussion entre le client et le serveur
	/*
	while(strcmp(ligne, "fin\n") != 0)
	{
		printf(">");
		if(fgets(ligne, LIGNE_MAX, stdin) == NULL)
			erreur("fegts null\n");
		lgEcr = ecrireLigne(soc, ligne);
		if(lgEcr == -1)
			erreur_IO("ecrireLigne");
		printf("Message envoyé au serveur\n");
	}*/
	nbRead = lireLigne(soc,ligne);
	if(nbRead == -1)
		erreur_IO("lireLigne");
	printf("Reception: %s", ligne);
		
	if (close(soc) == -1)
		erreur_IO("cloe");
	return 0;
}
