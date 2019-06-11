#include "pse.h"
#include "data_function.h"


#define BUF_SIZE 140
#define CMD "serveur"
#define MaxUser 10



enum states {NOT_CONNECTED, CONNECTION_REQUEST, REGISTER_REQUEST, CONNECTED};

enum events {JOINING_SERV, WAITING, SEND_CONNECTION_DEMAND, SEND_REGISTER_DEMAND, SEND_REGISTER_SUCCESS, SEND_CONNECTION_SUCCESS, SEND_PSEUDO_REQUEST, SEND_MDP_REQUEST,ERROR_MDP, PUBLISH, LIKE, FOLLOW, UNFOLLOW};

typedef struct {
	pthread_t Rid;
	pthread_t Wid;
	int tid;
	int canal;
	sem_t sem;
	sem_t sem_w;
	sem_t sem_r;
	enum states state;
	enum events event;
} Thread_Data;

struct sockaddr_in adrEcoute, adrClient;
Thread_Data thread_data[MaxUser];
sem_t sem_;

unsigned int lgAdrClient;
int ecoute, ret;

DataUtilisateurTete data_users;

void *receiveClient(void *arg)
{
	Thread_Data *tc = (Thread_Data*) arg;
	DataUtilisateur *courant;
	char buf[BUF_SIZE];
	int nbRead;
	int try_mdp = 5;
	char psdo[BUFFER_PSEUDO];
	int continu = 1;
	while (continu)
	{
		sem_wait(&tc->sem_r);
		printf("On arrive ici\n");
		
				switch(tc->event)
				{
					case (JOINING_SERV) :
						lireLigne(tc->canal,buf);
						if (strcmp(buf, "connection") == 0)
							tc->event = SEND_CONNECTION_DEMAND;
						else if (strcmp(buf, "register") == 0)
							tc->event = SEND_REGISTER_DEMAND;
						break;
					case (SEND_PSEUDO_REQUEST) :
						nbRead = lireLigne(tc->canal,buf);
						courant = findUserByPseudo(data_users.tete_users, buf);
						switch(tc->state)
						{
							case (CONNECTION_REQUEST) :
								if(courant != NULL)
								{
									printf("%s veut se connecter au serveur\n", courant->utilisateur->pseudo);
									tc->event = SEND_MDP_REQUEST;
								}
								else
								{
									//Utilisateur introuvable
									printf("erreur ... %s n'existe pas\n",buf);
								}
								break;
							case (REGISTER_REQUEST) :
								if(courant != NULL)
								{
									printf("%s déja pris\n", courant->utilisateur->pseudo);
								}
								else
								{
									strcpy(psdo, buf);
									printf("%s s'inscrit sur le serveur \n", buf);
									tc->event = SEND_MDP_REQUEST;
								}
								break;
						}
						break;
						
						
					case (SEND_MDP_REQUEST) :
						nbRead = lireLigne(tc->canal,buf);
						switch(tc->state)
						{
							case (CONNECTION_REQUEST) :
								if ( strcmp(buf, courant->utilisateur->mdp) == 0)
									tc->event = SEND_CONNECTION_SUCCESS;
								else 
								{
									try_mdp--;
									if (try_mdp < 0)
										tc->event = ERROR_MDP;
								}
								break;
							case (REGISTER_REQUEST) :
								if (nbRead > 0)
								{
									tc->event = SEND_REGISTER_SUCCESS;
									addUtilisateur(data_users.tete_users,psdo,buf,&data_users.info);
									courant = findUserByPseudo(data_users.tete_users, psdo);
								}
								break;
						}
						break;
					case (WAITING) :
						nbRead = lireLigne(tc->canal, buf);
						break;
				}		
				sem_post(&tc->sem_w);
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
			case (JOINING_SERV) :
			
				strcpy(buf,"Tapez \"connection\" pour se connecter ou \"register\" pour s'inscrire");
				break;
			case (SEND_CONNECTION_DEMAND) :
				switch (tc->state)
				{
					case (NOT_CONNECTED) :
						strcpy(buf, "Rentrez votre pseudo");
						tc->event = SEND_PSEUDO_REQUEST;
						tc->state = CONNECTION_REQUEST;
						break;
					default :
						strcpy(buf,"Action impossible");
						tc->event = WAITING;
						break;
				}
				break;
			case (SEND_REGISTER_DEMAND) :
				strcpy(buf, "Rentrez votre pseudo");
				tc->event = SEND_PSEUDO_REQUEST;
				tc->state = REGISTER_REQUEST;
				break;
			case (SEND_CONNECTION_SUCCESS) : 
				switch(tc->state)
				{
					case (CONNECTION_REQUEST) :
						strcpy(buf, "Bienvenue vous êtes connecté");
						tc->state = CONNECTED;
						tc->event = WAITING;
						break;
				}	
				break;
			case (SEND_REGISTER_SUCCESS) :
				strcpy(buf, "Bienvenue votre inscription a fonctionné");
				tc->state = CONNECTED;
				tc->event = WAITING;
				break;
			case (SEND_MDP_REQUEST) :
				switch(tc->state)
				{
					case (CONNECTION_REQUEST) :
						strcpy(buf,"Tapez votre mot de passe");
						break;
					case (REGISTER_REQUEST) :
						strcpy(buf,"Choisissez votre mot de passe");
						break;
				}
				break;
			case (ERROR_MDP) :
				strcpy(buf,"Trop d'échecs, vous allez être déconnecté");
				tc->event = WAITING;
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
	//On initialise la tête
	data_users.tete_users = (DataUtilisateur*) malloc(sizeof(DataUtilisateur));
	initDataUtilisateur(data_users.tete_users);
	//On initialise les infos
	data_users.info.nb_utilisateur = 0;
	data_users.info.nb_publication = 0;
	//On charge les utilisateurs
	loadDataFromFile(&data_users);
	//On affiche toutes les données
	printData(data_users.tete_users);
	addUtilisateur(data_users.tete_users, "Vivien", "mdp", &data_users.info);
	printData(data_users.tete_users);
	
	//Création du thread de sauvegarde
	if (pthread_create(&data_users.info.id_thread_save, NULL, autoSave, &data_users) != 0) 
		erreur_IO("creation of saving thread");
	//debug et test
	int id_last_user = getLastUserId(&data_users);
	printf("%d", id_last_user);
	short port;
	
	if (argc != 2)
		erreur("Mauvaise syntaxe : %s port\n", argv[0]);
	
	port = (short) atoi(argv[1]);
	sem_init( &sem_, 0, MaxUser);
	for (int i = 0; i < MaxUser; i++)
	{
		thread_data[i].tid = i;
		thread_data[i].state = NOT_CONNECTED;
		thread_data[i].event = JOINING_SERV;
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


