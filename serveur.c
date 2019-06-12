#include "pse.h"
#include "data_function.h"


#define BUF_SIZE 400
#define CMD "serveur"
#define MaxUser 10



enum states {NOT_JOINED,NOT_CONNECTED,TRY_CONNECTION, CONNECTION_REQUEST, REGISTER_REQUEST, CONNECTION_REQUEST_PSEUDO, REGISTER_REQUEST_PSEUDO, CONNECTION_REQUEST_MDP, REGISTER_REQUEST_MDP, CHECK_CONNECTION, CHECK_REGISTER, CONNECTED, WAIT_FOR_COMMAND};

enum events {LEAVING_SERV, JOINING_SERV, SEND_CONNECTION_DEMAND, SEND_REGISTER_DEMAND, SEND_PSEUDO_REQUEST, SEND_MDP_REQUEST, SEND_CONNECTION_FAILED, SEND_CONNECTION_SUCCESS, SEND_HOMEPAGE, PUBLISH, VIEW_STAT, LIKE, FOLLOW, UNFOLLOW};

typedef struct {
	pthread_t Rid;
	pthread_t Wid;
	int tid;
	int canal;
	sem_t sem;
	sem_t sem_w;
	sem_t sem_r;
	int lec_on;
	enum states state;
	enum events event;
	DataUtilisateur* user;
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
	char buf[BUF_SIZE];
	int nbRead;
	int try_mdp = 5;
	char psdo[BUFFER_PSEUDO];
	int continu = 1;
	while (continu )
	{
		sem_wait(&tc->sem_r);
		printf("On arrive ici\n");
		while(tc->canal != -1)
		{
			if (tc->lec_on)
			{
				if ((nbRead =lireLigne(tc->canal,buf)) <= 0)
				{
					tc->canal = -1;
					break;
				}
				if (strcmp(buf,"fin") == 0)	//Vérification à chaque entrée si l'utilisateur veut quitter
				{
					tc->event = LEAVING_SERV;
					sem_post(&tc->sem_w);
					break;
				}
			}
			switch(tc->state)
			{
				case (NOT_CONNECTED) :
					tc->event = JOINING_SERV;
					break;
				case (TRY_CONNECTION) :
					if ( strcmp(buf,"/connection") == 0)
						tc->event = SEND_CONNECTION_DEMAND;
					else if ( strcmp(buf, "/register") == 0)
						tc->event = SEND_REGISTER_DEMAND;
					break;
				case (CONNECTION_REQUEST) :
					tc->event = SEND_PSEUDO_REQUEST;
					break;
				case (REGISTER_REQUEST) :
					tc->event = SEND_PSEUDO_REQUEST;
					break;
				case (CONNECTION_REQUEST_PSEUDO) :
					tc->user = findUserByPseudo(data_users.tete_users, buf);
					if(tc->user != NULL)
					{
						printf("%s veut se connecter au serveur\n", tc->user->utilisateur->pseudo);
						tc->event = SEND_MDP_REQUEST;
					}
					else
					{
						//Utilisateur introuvable
						printf("erreur ... %s n'existe pas\n",buf);
					}
					break;
				case (REGISTER_REQUEST_PSEUDO) :
					tc->user = findUserByPseudo(data_users.tete_users, buf);
					if(tc->user != NULL)
					{
						printf("%s déja pris\n", tc->user->utilisateur->pseudo);
					}
					else
					{
						strcpy(psdo, buf);
						printf("%s s'inscrit sur le serveur \n", buf);
						tc->event = SEND_MDP_REQUEST;
					}
					break;
				case (CONNECTION_REQUEST_MDP) :
					if ( strcmp(buf, tc->user->utilisateur->mdp) == 0)
						tc->event = SEND_CONNECTION_SUCCESS;
					else 
					{
						try_mdp--;
						if (try_mdp < 0)
							tc->event = SEND_CONNECTION_FAILED;
					}
					break;
				case (REGISTER_REQUEST_MDP) :
					if (nbRead > 0)
					{
						tc->event = SEND_CONNECTION_SUCCESS;
						addUtilisateur(data_users.tete_users,psdo,buf,&data_users.info);
						tc->user = findUserByPseudo(data_users.tete_users, psdo);
					}
					else
						tc->event = SEND_CONNECTION_FAILED;
					break;
				case (CONNECTED) : 
					tc->event = SEND_HOMEPAGE;
					break;
				case (WAIT_FOR_COMMAND) :
					if (strcmp(buf,"/publish") == 0 || strcmp(buf,"/p") == 0)
					{
						tc->event = PUBLISH;
					}
					else if (strcmp(buf,"/stat") == 0 || strcmp(buf,"/s") == 0)
					{
						tc->event = VIEW_STAT;
					}
					if (strcmp(buf,"/follow") == 0 || strcmp(buf,"/f") == 0)
					{
						tc->event = FOLLOW;
					}
			}
			sem_post(&tc->sem_w);
			sem_wait(&tc->sem_r);
		}
	}
}

void *sendClient(void *arg)
{
	Thread_Data *tc = (Thread_Data*) arg;
	char buf[BUF_SIZE];
	char ans_client[BUF_SIZE];
	int nbRead;
	int continu =1;
	while (continu)
	{
		sem_wait(&tc->sem_w);
		while (tc->canal != -1)
		{
			switch(tc->event)
			{
				case (JOINING_SERV) :
				
					strcpy(ans_client,"1");
					strcpy(buf,"Tapez \"/connection\" pour se connecter ou \"/register\" pour s'inscrire");
					tc->state = TRY_CONNECTION;
					break;
	
				case (SEND_CONNECTION_DEMAND) :
					strcpy(ans_client,"0");
					strcpy(buf, "Connexion");
					tc->state = CONNECTION_REQUEST;
					break;
				case (SEND_REGISTER_DEMAND) :
					strcpy(ans_client,"0");
					strcpy(buf, "Inscription");
					tc->state = REGISTER_REQUEST;
					break;
				case (SEND_PSEUDO_REQUEST) :
					strcpy(ans_client,"1");
					switch(tc->state)
					{
						case (CONNECTION_REQUEST) :
							strcpy(buf,"Tapez votre pseudo");
							tc->state = CONNECTION_REQUEST_PSEUDO;
							break;
						case (REGISTER_REQUEST) :
							strcpy(buf,"Choisissez votre pseudo");
							tc->state = REGISTER_REQUEST_PSEUDO;
							break;
					}
					break;
				case (SEND_MDP_REQUEST) :
					strcpy(ans_client,"1");
					switch(tc->state)
					{
						case (CONNECTION_REQUEST_PSEUDO) :
							strcpy(buf,"Tapez votre mot de passe");
							tc->state = CONNECTION_REQUEST_MDP;
							break;
						case (REGISTER_REQUEST_PSEUDO) :
							strcpy(buf,"Choisissez votre mot de passe");
							tc->state = REGISTER_REQUEST_MDP;
							break;
					}
					break;
				case (SEND_CONNECTION_SUCCESS) : 
					strcpy(ans_client,"0");
					switch(tc->state)
					{
						case (CONNECTION_REQUEST_MDP) :
							strcpy(buf, "Bienvenue vous êtes connecté");
							tc->state = CONNECTED;
							break;
						case (REGISTER_REQUEST_MDP) :
							strcpy(buf, "Bienvenue votre inscription a fonctionné");
							tc->state = CONNECTED;
							break;
					}	
					break;
				case (SEND_CONNECTION_FAILED) :
					strcpy(ans_client,"0");
					strcpy(buf,"Trop d'échecs, vous allez être déconnecté");
					tc->event = JOINING_SERV;
					tc->state = NOT_CONNECTED;
					tc->user = NULL;
					break;
				case (SEND_HOMEPAGE) :
					strcpy(ans_client,"1");
					strcpy(buf,"Bienvenue sur ZETIR ! Le réseau social du futur ! Partagez votre vie privée qui n'interressent personne avec tous vos abonnés !");
					tc->state = WAIT_FOR_COMMAND;
					break;
			}
			if (strcmp(ans_client,"1") == 0)
				tc->lec_on = 1;
			else 
				tc->lec_on = 0;
			printf("On envoie un message %d\n", tc->canal);
			ecrireLigne(tc->canal, ans_client);
			ecrireLigne(tc->canal, buf);
			sem_post(&tc->sem_r);
			sem_wait(&tc->sem_w);
		}
	}
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
		thread_data[i].state = NOT_JOINED;
		thread_data[i].event = JOINING_SERV;
		thread_data[i].canal = -1;
		thread_data[i].lec_on = 1;
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
		thread_data[libre].state = NOT_CONNECTED;
		sem_post(&thread_data[libre].sem_w);
		printf("Lancement de la discussion client/serveur\n");
	}
	exit(EXIT_SUCCESS);
}


