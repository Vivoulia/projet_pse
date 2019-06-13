#include "pse.h"
#include "data_function.h"


#define BUF_SIZE 400
#define CMD "serveur"
#define MaxUser 1



enum states {NOT_JOINED,NOT_CONNECTED,TRY_CONNECTION, CONNECTION_REQUEST, REGISTER_REQUEST, CONNECTION_REQUEST_PSEUDO, REGISTER_REQUEST_PSEUDO, CONNECTION_REQUEST_MDP, REGISTER_REQUEST_MDP, CHECK_CONNECTION, CHECK_REGISTER, CONNECTED, ACCUEIL, WAIT_FOR_COMMAND, PUBLISHING, DELETE_REQUEST, PROFILE};

enum events {LEAVING_SERV, JOINING_SERV, SEND_CONNECTION_DEMAND, SEND_REGISTER_DEMAND, SEND_PSEUDO_REQUEST, SEND_MDP_REQUEST, SEND_CONNECTION_FAILED, SEND_CONNECTION_SUCCESS, SEND_HOMEPAGE, SEND_ACCUEIL, PUBLISH,SEND_PUBLICATION, VIEW_PUBLI, SHOW_OUR_PROFILE, SHOW_PROFILE, SHOW_NEWS, LIKE, FOLLOW, UNFOLLOW, DELETE_PUBLI, SEND_DELETE, DESAC, HELP, DISCONNECT, ERROR_PSEUDO, ERROR_MDP, ERROR_COMMAND, ERROR_COMMAND2, ERROR_COMMAND3, ERROR_COMMAND4, ERROR_PUBLI, ERROR_FOLLOW, ERROR_FOLLOW2, ERROR_DELETE, ERROR_UNFOLLOW};

typedef struct {
	pthread_t Rid;
	pthread_t Wid;
	int tid;
	int canal;
	sem_t sem;
	sem_t sem_w;
	sem_t sem_r;
	int lec_on;
	char buffy[BUF_SIZE];
	enum states state;
	enum events event;
	DataUtilisateur* user;
} Thread_Data;

struct sockaddr_in adrEcoute, adrClient;
Thread_Data thread_data[MaxUser];
sem_t sem_;

unsigned int lgAdrClient;
int ecoute, ret;
int continu = 1;

DataUtilisateurTete data_users;


void *receiveClient(void *arg)	//THREAD DE RECEPTION DE DONNEES DE L'UTILISATEUR
{
	Thread_Data *tc = (Thread_Data*) arg;
	char buf[BUF_SIZE];
	char *x;
	char command[BUF_SIZE];
	char publi[BUFFER_PUBLI];
	int nbRead;
	int try_mdp = 4;
	char psdo[BUFFER_PSEUDO];
	int r = 0;
	while (continu )
	{
		try_mdp = 4;
		sem_wait(&tc->sem_r);
		//printf("On arrive ici\n"); MESSAGE DEBUG
		while(tc->canal != -1)
		{
			if (tc->lec_on)	//Activation ou non de la lecture de la réponse client
			{
				if ((nbRead =lireLigne(tc->canal,buf)) <= 0)
				{
					tc->canal = -1;
					break;
				}
				if (strcmp(buf,"/fin") == 0)	//Vérification à chaque entrée si l'utilisateur veut quitter
				{
					tc->event = LEAVING_SERV;
					break;
				}
			}
			switch(tc->state)	
			{
				case (NOT_CONNECTED) :	//Lancement de l'application par l'utilisateur
					tc->event = JOINING_SERV;
					break;
					
				case (TRY_CONNECTION) :	//Choix entre connexion et inscription pour l'utilisateur
					if ( strcmp(buf,"/connection") == 0)
						tc->event = SEND_CONNECTION_DEMAND;
					else if ( strcmp(buf, "/register") == 0)
						tc->event = SEND_REGISTER_DEMAND;
					break;
					
				case (CONNECTION_REQUEST) :	//L'utilisateur a chosi connexion
					tc->event = SEND_PSEUDO_REQUEST;
					break;
					
				case (REGISTER_REQUEST) :	//L'utilisateur a choisi inscription
					tc->event = SEND_PSEUDO_REQUEST;
					break;
					
				case (CONNECTION_REQUEST_PSEUDO) :	//Demande du pseudo pour la connexion
					tc->user = findUserByPseudo(data_users.tete_users, buf);
					if(tc->user != NULL)	
					{
						printf("%s veut se connecter au serveur\n", tc->user->utilisateur->pseudo);
						tc->event = SEND_MDP_REQUEST;	//L'utilisateur existe bien -> demande de mot de passe
					}
					else
					{
						//Utilisateur introuvable
						printf("erreur ... %s n'existe pas\n",buf);
						tc->event = ERROR_PSEUDO;
					}
					break;
					
				case (REGISTER_REQUEST_PSEUDO) :	//Demande du pseudo pour l'inscription
					tc->user = findUserByPseudo(data_users.tete_users, buf);
					if(tc->user != NULL)
					{
						printf("%s déja pris\n", tc->user->utilisateur->pseudo);
						tc->event = ERROR_PSEUDO;	//Pseudo déjà utilisé par un autre utilisateur
					}
					else
					{
						strcpy(psdo, buf);	//Pseudo disponible -> demande de selection de mot de passe
						printf("%s s'inscrit sur le serveur \n", buf);
						tc->event = SEND_MDP_REQUEST;
					}
					break;
					
				case (CONNECTION_REQUEST_MDP) :	//demande de mot de passe lors de la connexion
					if ( strcmp(buf, tc->user->utilisateur->mdp) == 0)	// Mot de passe valide
						tc->event = SEND_CONNECTION_SUCCESS;
					else 
					{	//Mauvais mot de passe
						try_mdp--;
						printf("%d\n",try_mdp);
						if (try_mdp < 0)
							tc->event = SEND_CONNECTION_FAILED;
						else tc->event = ERROR_MDP;	//Trop d'erreur pour le mot de passe
					}
					break;
				case (REGISTER_REQUEST_MDP) :	//Demande de mot de passe lors de l'inscription
					if (nbRead > 0)
					{
						tc->event = SEND_CONNECTION_SUCCESS;	//le mot de passe est valide
						addUtilisateur(data_users.tete_users,psdo,buf,&data_users.info);
						tc->user = findUserByPseudo(data_users.tete_users, psdo);
					}
					else
						tc->event = ERROR_MDP;	//Erreur dans la selection de mot de passe
					break;
					
				case (CONNECTED) : //Connecté à un compte
					tc->event = SEND_HOMEPAGE;
					break;
					
				case (ACCUEIL) :	//Accueil du serveur
					tc->event = SEND_ACCUEIL;
					break;
					
				case (WAIT_FOR_COMMAND) :	//Lecture des commandes envoyées par le client
					x =strtok(buf," ");
					if (x == NULL)
						break;
					strcpy(command,x);
					if (strcmp(command,"/publish") == 0 || strcmp(command,"/p") == 0)
					{
						tc->event = PUBLISH;	//Publication d'un message
					}
					else if (strcmp(command,"/help") == 0 || strcmp(command,"/h") == 0)
					{
						tc->event = HELP;	//Affichage des commandes 
					}
					else if (strcmp(command,"/profile") == 0 || strcmp(command,"/pr") == 0)
					{
						x = strtok(NULL," ");
						if (x == NULL)
						{
							tc->event = SHOW_OUR_PROFILE;	//Affichage du profil de l'utilisateur
							break;
						}
						strcpy(tc->buffy,x);	//Affichage d'un autre profil
						tc->event = SHOW_PROFILE;
					}
					else if (strcmp(command,"/follow") == 0 || strcmp(command,"/f") == 0)
					{
						x = strtok(NULL," ");
						if (x == NULL)
						{
							tc->event = ERROR_COMMAND2;	//Mauvaise utilisation de /follow
							break;
						}
						strcpy(psdo,x);
						r = addAbonnementByPseudo(&data_users ,tc->user, psdo );	//Abonnement
						if ( r == 1)
						{
							tc->event = ERROR_FOLLOW;	//Si l'utilisateur n'existe pas
							break;
						}
						else if (r == 2)
						{
							tc->event = ERROR_FOLLOW2;	//si on est déjà abonné
							break;
						}
						strcpy(tc->buffy,psdo);
						//printData(data_users.tete_users);	//Affichage côté serveur (utile au debug)
						tc->event = FOLLOW;
					}
					else if (strcmp(command,"/unfollow") == 0 || strcmp(command,"/u") == 0)
					{
						x = strtok(NULL," ");
						if (x == NULL)
						{
							tc->event = ERROR_COMMAND3;	//Mauvaise utilisation
							break;
						}
						strcpy(psdo,x);
						r = deleteAbonnementByPseudo(&data_users ,tc->user, psdo);	//Desabonnement
						if (r ==1)
						{
							tc->event = ERROR_UNFOLLOW;	//Non abonné ou n'existe pas
							break;
						}
						strcpy(tc->buffy,psdo);
						//printData(data_users.tete_users); AFFICHAGE UTILE AU DEBUG
						tc->event = UNFOLLOW;
					}
					else if (strcmp(command, "/disconnect") == 0 || strcmp(command,"/d") == 0)
					{
						tc->event = DISCONNECT;	//Deconnexion du compte
					}
					else if (strcmp(command, "/delete") == 0 || strcmp(command,"/del") == 0)
					{
						x = strtok(NULL," ");	//Suppression de publication
						if (x == NULL)
						{
							tc->event = ERROR_COMMAND4;
							break;
						}
						r = strtol(x,(char**) NULL, 10);
						if (r >= tc->user->nb_publication)
						{
							tc->event = ERROR_DELETE;
							break;
						}
						Publication* current_pub = tc->user->publication;
						while (current_pub->id != r)
							current_pub = current_pub->suiv;	//Recherche de la publication pour afficher celle que l'on veut supprimer
						strcpy(tc->buffy,current_pub->texte);
						tc->event = DELETE_PUBLI;
					}
					else if (strcmp(command, "/news") == 0 || strcmp(command,"/n") == 0)
					{
					 	tc->event = SHOW_NEWS;	//Affichage du fil d'actualité
					}
					else if ( command[0]== 47) // (= '/')
					{
						tc->event = ERROR_COMMAND;	//COmmande inconnue
					}
					break;
					
				case (PUBLISHING) :	//Publication
					if (nbRead <= 0)
						tc->event = ERROR_PUBLI;
					strcpy(publi,buf);
					strcat(publi,"\n");	//Ajout d'un retour à la ligne pour l'enregistrement
					addNewPublication(data_users.tete_users, tc->user->utilisateur->id,publi);
					//printData(data_users.tete_users);	AFFICHAGE UTILE AU DEBUG
					tc->event = SEND_PUBLICATION;
					break;
					
				case (DELETE_REQUEST) :
					if (strcmp(buf,"o") == 0)
					{
						tc->event = SEND_DELETE;	//Suppression de publication
						deletePublicationById(tc->user,r);
						//printData(data_users.tete_users); AFFICHAGE UTILE AU DEBUG
					}
					else if (strcmp(buf,"n") == 0)	//Si l'utilisateur ne veut plus la supprimer
						tc->event = SEND_ACCUEIL;
					break;
					
				default :
					break;
			}
			sem_post(&tc->sem_w);	//Le thread d'écriture peut y aller
			sem_wait(&tc->sem_r);	//On attend le signal du thread d'écriture pour repartir en lecture
		}
		sem_post(&tc->sem_w);
	}
	exit(EXIT_SUCCESS);
}




void *sendClient(void *arg)		//THREAD D'ENVOI DE DONNEES À L'UTILISATEUR
{
	Thread_Data *tc = (Thread_Data*) arg;
	char buf[BUF_SIZE];
	char ans_client[BUF_SIZE];
	DataUtilisateur *profile_user;
	Publication *current_publi;
	UtilisateurChaine* current_abo;
	while (continu)
	{
		sem_wait(&tc->sem_w);
		while (tc->canal != -1)
		{
			switch(tc->event)
			{
				case (JOINING_SERV) :	//Connexion au serveur
				
					strcpy(ans_client,"1");
					strcpy(buf,"Tapez \"/connection\" pour se connecter ou \"/register\" pour s'inscrire");
					tc->state = TRY_CONNECTION;
					break;
					
				case (LEAVING_SERV) :	//Lorsque l'utilisateur veut quitter l'application
					printf("%s : Deconnexion du socket du thread %d\n",CMD,tc->tid);
					strcpy(ans_client,"0");
					strcpy(buf,"");
					tc->event = JOINING_SERV;
					tc->state = NOT_JOINED;
					tc->user = NULL;
					close(tc->canal);
					sem_post(&sem_);
					tc->canal = -1;
					break;
					
				case (SEND_CONNECTION_DEMAND) :	//Demande de connexion
					strcpy(ans_client,"0");
					strcpy(buf, "Connexion");
					tc->state = CONNECTION_REQUEST;
					break;
					
				case (SEND_REGISTER_DEMAND) :	//Demande d'inscription
					strcpy(ans_client,"0");
					strcpy(buf, "Inscription");
					tc->state = REGISTER_REQUEST;
					break;
					
				case (SEND_PSEUDO_REQUEST) :	//Envoi demande de pseudo (dépend de la requete précédente)
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
						default :
							break;
					}
					break;
					
				case (SEND_MDP_REQUEST) :	//Envoi demande mot de passe
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
						default :
							break;
					}
					break;
					
				case (SEND_CONNECTION_SUCCESS) : //Envoi reussite de connexion
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
						default :
							break;
					}	
					break;
					
				case (SEND_CONNECTION_FAILED) :	//Echec de connexion
					strcpy(ans_client,"0");
					strcpy(buf,"Trop d'échecs, vous allez être déconnecté");
					tc->event = JOINING_SERV;
					tc->state = NOT_CONNECTED;
					tc->user = NULL;
					break;
					
				case (SEND_HOMEPAGE) :	//Petite présentation de l'application
					strcpy(ans_client,"0");
					strcpy(buf,"Bienvenue sur ZETIR ! Le réseau social du futur ! Partagez votre vie privée qui n'interressent personne avec tous vos abonnés !");
					tc->state = ACCUEIL;
					break;
					
				case (SEND_ACCUEIL) :	//Page d'accueil de l'application
					strcpy(ans_client,"1");
					strcpy(buf,"Accueil");
					tc->state = WAIT_FOR_COMMAND;
					break;
					
				case (PUBLISH) :	//Envoi de la demande pour créer la publication
					strcpy(ans_client,"1");
					strcpy(buf,"Tapez le contenu de votre publication (140 caractères maximum)");
					tc->state = PUBLISHING;
					break;
					
				case (SEND_PUBLICATION) :	//Si la publication a réussi
					strcpy(ans_client,"0");
					strcpy(buf,"Publication crée");
					tc->state = ACCUEIL;
					break;
					
				case (VIEW_PUBLI) :	//WIP
					strcpy(ans_client,"0");
					tc->state = ACCUEIL;
					break;
					
				case (FOLLOW) :	//Réussite de l'abonnement
					strcpy(ans_client,"0");
					strcpy(buf,"Abonnement réussi à ");
					strcat(buf,tc->buffy);
					tc->state = ACCUEIL;
					break;
					
				case (UNFOLLOW) :	//Réussite du désabonnement
					strcpy(ans_client,"0");
					strcpy(buf,"Désabonnement réussi de");
					strcat(buf,tc->buffy);
					tc->state = ACCUEIL;
					break;
					
				case (DELETE_PUBLI) :	//Demande de suppression
					strcpy(ans_client,"0");
					strcpy(buf,"'");
					strcat(buf,tc->buffy);
					strcat(buf,"'");
					ecrireLigne(tc->canal,ans_client);
					ecrireLigne(tc->canal,buf);
					strcpy(ans_client,"1");
					strcpy(buf,"Etes-vous sûr de vouloir supprimer cette publication ? (o/n)");
					tc->state = DELETE_REQUEST;
					break;
					
				case (SEND_DELETE) :	//Réussite de suppression
					strcpy(ans_client,"1");
					strcpy(buf,"Publication supprimée");
					tc->state = ACCUEIL;
					break;
					
				case (SHOW_OUR_PROFILE) :	//Affichage de son profil
					strcpy(ans_client,"0");
					sprintf(buf,"Profil de %s : %d publications | %d abonnés | %d abonnement",tc->user->utilisateur->pseudo,tc->user->nb_publication,tc->user->nb_abonne,tc->user->nb_abonnement);
					ecrireLigne(tc->canal,ans_client);
					ecrireLigne(tc->canal,buf);
					current_abo = tc->user->abonnements;
					while (current_abo != NULL)
					{
						sprintf(buf,"	Abonnements: %s", current_abo->data_user->utilisateur->pseudo);
						current_abo = current_abo->suiv;
						ecrireLigne(tc->canal,ans_client);
						ecrireLigne(tc->canal,buf);
					}
					current_abo = tc->user->abonnes;
					while (current_abo != NULL)
					{
						sprintf(buf,"	Abonnes: %s", current_abo->data_user->utilisateur->pseudo);
						current_abo = current_abo->suiv;
						ecrireLigne(tc->canal,ans_client);
						ecrireLigne(tc->canal,buf);
					}
					current_publi = tc->user->publication;
					while (current_publi != NULL)
					{
						sprintf(buf,"	Publication id %d: le: %d/%d/%d a %dh%dmin%ds\n", current_publi->id, current_publi->date->tm_mday, current_publi->date->tm_mon+1, current_publi->date->tm_year+1900,current_publi->date->tm_hour, current_publi->date->tm_min, current_publi->date->tm_sec);
						ecrireLigne(tc->canal,ans_client);
						ecrireLigne(tc->canal,buf);
						sprintf(buf,"		%s\n", current_publi->texte);
						ecrireLigne(tc->canal,ans_client);
						ecrireLigne(tc->canal,buf);
						current_publi = current_publi->suiv;
					}
					strcpy(ans_client,"1");
					strcpy(buf,"Fin affichage profil");
					tc->state = WAIT_FOR_COMMAND;
					break;
					
				case (SHOW_PROFILE) :	//Affichage du profil
					profile_user = findUserByPseudo(data_users.tete_users,tc->buffy);
					strcpy(ans_client,"0");
					sprintf(buf,"Profil de %s : %d publications | %d abonnés | %d abonnement",profile_user->utilisateur->pseudo,profile_user->nb_publication,profile_user->nb_abonne,profile_user->nb_abonnement);
					current_abo = profile_user->abonnements;
					while (current_abo != NULL)
					{
						sprintf(buf,"	Abonnements: %s", current_abo->data_user->utilisateur->pseudo);
						current_abo = current_abo->suiv;
						ecrireLigne(tc->canal,ans_client);
						ecrireLigne(tc->canal,buf);
					}
					current_abo = profile_user->abonnes;
					while (current_abo != NULL)
					{
						sprintf(buf,"	Abonnes: %s\n", current_abo->data_user->utilisateur->pseudo);
						current_abo = current_abo->suiv;
						ecrireLigne(tc->canal,ans_client);
						ecrireLigne(tc->canal,buf);
					}
					current_publi = profile_user->publication;
					while (current_publi != NULL)
					{
						sprintf(buf,"	Publication id %d: le: %d/%d/%d a %dh%dmin%ds\n", current_publi->id, current_publi->date->tm_mday, current_publi->date->tm_mon+1, current_publi->date->tm_year+1900,current_publi->date->tm_hour, current_publi->date->tm_min, current_publi->date->tm_sec);
						ecrireLigne(tc->canal,ans_client);
						ecrireLigne(tc->canal,buf);
						sprintf(buf,"		%s\n", current_publi->texte);
						ecrireLigne(tc->canal,ans_client);
						ecrireLigne(tc->canal,buf);
						current_publi = current_publi->suiv;
					}
					strcpy(ans_client,"1");
					strcpy(buf,"Fin affichage profil");
					tc->state = WAIT_FOR_COMMAND;
					break;

				case (SHOW_NEWS) :	//Fil d'actualité (10 dernieres publications)
					strcpy(ans_client,"0");
					UtilisateurChaine* abonnement = tc->user->abonnements;
					if(abonnement == NULL)
					{
						strcpy(buf,"Aucun news a afficher");
						//Aucun news (pas d'abonnements)
					}
					else
					{
						int nb_publi_affiche = 0;
						int stop = 0;
						int* tab_abo =(int*) malloc(tc->user->nb_abonnement*sizeof(int));
						for (int i = 0; i<tc->user->nb_abonnement; i++)
						{
							tab_abo[i] = 0;
						}
						while(nb_publi_affiche < 10 && stop == 0)
						{
							Publication* recent_publi = NULL;
							UtilisateurChaine* recent_abonnement = NULL;
							UtilisateurChaine* current_abonnement = tc->user->abonnements;
							int i_recent_abonnement = 0;
							int i_current_abonnement = 0;
							while(current_abonnement != NULL)
							{
								if (tab_abo[i_current_abonnement] != -1)
								{
									Publication* current_publi = current_abonnement->data_user->publication;
									for(int i = 0; i<tab_abo[i_current_abonnement]; i++)
									{
										if(current_publi == NULL)
										{
											tab_abo[i_current_abonnement] = -1;
										}
										else
										{
											current_publi = current_publi->suiv;
										}
									}
									if(current_publi != NULL)
									{
										if(recent_publi == NULL || isMoreRecent(current_publi, recent_publi) == 1)
										{
											recent_publi = current_publi;
											recent_abonnement = current_abonnement;
											i_recent_abonnement = i_current_abonnement;
										}
									}
								}
								current_abonnement = current_abonnement->suiv;
								i_current_abonnement++;
							}
							if(recent_publi == NULL)
							{
								stop = 1;
							}
							else
							{
								printf("recent_publi de %s:%s\n", recent_abonnement->data_user->utilisateur->pseudo, recent_publi->texte);
								strcpy(ans_client,"0");
								strcpy(buf,"#############");
								ecrireLigne(tc->canal, ans_client);
								ecrireLigne(tc->canal, buf);
								
								sprintf(buf, "Publication de: %s", recent_abonnement->data_user->utilisateur->pseudo);
								ecrireLigne(tc->canal, ans_client);
								ecrireLigne(tc->canal, buf);
								
								sprintf(buf, "Le %d/%d/%d a %dh%dmin" ,recent_publi->date->tm_mday, recent_publi->date->tm_mon+1, recent_publi->date->tm_year+1900, 
																		recent_publi->date->tm_hour, recent_publi->date->tm_min);
								ecrireLigne(tc->canal, ans_client);
								ecrireLigne(tc->canal, buf);
								
								sprintf(buf, "	%s", recent_publi->texte);
								ecrireLigne(tc->canal, ans_client);
								ecrireLigne(tc->canal, buf);
								
								nb_publi_affiche++;
								tab_abo[i_recent_abonnement]++;

							}
						}
						free(tab_abo);
					}
					strcpy(buf,"#############");
					strcpy(ans_client,"1");
					tc->state = WAIT_FOR_COMMAND;
					break;
					
				case (HELP) :	//Affichage des commandes serveurs
					strcpy(ans_client,"0");
					strcpy(buf,"Liste des commandes :");
					ecrireLigne(tc->canal, ans_client);
					ecrireLigne(tc->canal, buf);
					strcpy(buf,"/publish (/p) : Publier un message");
					ecrireLigne(tc->canal, ans_client);
					ecrireLigne(tc->canal, buf);
					strcpy(buf,"/follow *pseudo* (/f *pseudo*) : S'abonner à un utilisateur");
					ecrireLigne(tc->canal, ans_client);
					ecrireLigne(tc->canal, buf);
					strcpy(buf,"/unfollow *pseudo* (/u *pseudo*) : Se désabonner d'un utilisateur");
					ecrireLigne(tc->canal, ans_client);
					ecrireLigne(tc->canal, buf);
					strcpy(buf,"/profile [pseudo] (/pr [pseudo]) : Voir son profil ou celui de [pseudo]");
					ecrireLigne(tc->canal, ans_client);
					ecrireLigne(tc->canal, buf);
					strcpy(buf,"/news (/n) : Afficher le fil d'actualité");
					ecrireLigne(tc->canal, ans_client);
					ecrireLigne(tc->canal, buf);
					strcpy(buf,"/delete *Id_publication* (/del *Id_publication*) : Supprimer la publication avec l'identifiant Id_publication (un /profile permet de récuperer les identifiants)");
					ecrireLigne(tc->canal, ans_client);
					ecrireLigne(tc->canal, buf);
					strcpy(buf,"/disconnect (/d) : Se deconnecter");
					ecrireLigne(tc->canal, ans_client);
					ecrireLigne(tc->canal, buf);
					strcpy(buf,"/fin : Quitter l'application");
					strcpy(ans_client,"1");
					tc->state = WAIT_FOR_COMMAND;
					break;
					
				case (DISCONNECT) :	//Deconnexion de l'utilisateur
					strcpy(ans_client,"0");
					strcpy(buf,"Deconnexion");
					tc->state = NOT_CONNECTED;
					tc->user = NULL;
					break;
				//---------------------------------------------------
				//				Gestion des erreurs					|
				//---------------------------------------------------
				case (ERROR_PSEUDO) :
					strcpy(ans_client, "0");
					switch(tc->state)
					{
						case (CONNECTION_REQUEST_PSEUDO) :
							strcpy(buf,"Cet utilisateur n'existe pas");
							tc->state = CONNECTION_REQUEST;
							break;
						case (REGISTER_REQUEST_PSEUDO) :
							strcpy(buf,"Ce pseudo est déjà pris");
							tc->state = REGISTER_REQUEST;
							break;
						default :
							break;
					}
					break;
					
				case (ERROR_MDP) :
					strcpy(ans_client, "1");
					switch(tc->state)
					{
						case (CONNECTION_REQUEST_MDP) :
							strcpy(buf,"Mot de passe erroné");
							tc->event = SEND_MDP_REQUEST;
							break;
						case (REGISTER_REQUEST_MDP) :
							tc->event = SEND_MDP_REQUEST;
							strcpy(buf,"Mot de passe invalide");
							break;
						default :
							break;
					}
					break;
					
				case (ERROR_COMMAND) :
					strcpy(ans_client,"1");
					strcpy(buf,"Commande inconnue. Tapez \"/help\" pour afficher les commandes disponibles");
					tc->state = WAIT_FOR_COMMAND;
					break;
					
				case (ERROR_COMMAND2) :
					strcpy(ans_client,"1");
					strcpy(buf,"Mauvaise syntaxe, la bonne est la suivante : /follow *pseudo*");
					tc->state = WAIT_FOR_COMMAND;
					break;
					
				case (ERROR_COMMAND3) :
					strcpy(ans_client,"1");
					strcpy(buf,"Mauvaise syntaxe, la bonne est la suivante : /unfollow *pseudo*");
					tc->state = WAIT_FOR_COMMAND;
					break;
					
				case (ERROR_COMMAND4) :
					strcpy(ans_client,"1");
					strcpy(buf,"Mauvaise syntaxe, la bonne est la suivante : /delete *ID_publication*");
					tc->state = WAIT_FOR_COMMAND;
					break;
					
				case (ERROR_PUBLI) :
					strcpy(ans_client,"0");
					strcpy(buf,"Impossible de publier ce message");
					tc->state = ACCUEIL;
					break;
					
				case (ERROR_FOLLOW) :
					strcpy(ans_client,"0");
					strcpy(buf,"Cet utilisateur n'existe pas");
					tc->state = ACCUEIL;
					break;
					
				case (ERROR_FOLLOW2) :
					strcpy(ans_client,"0");
					strcpy(buf,"Vous êtes déjà abonné à cet utilisateur");
					tc->state = ACCUEIL;
					break;
					
				case (ERROR_UNFOLLOW) :
					strcpy(ans_client,"0");
					strcpy(buf,"Vous n'êtes pas abonné à cet utilisateur");
					tc->state = ACCUEIL;
					break;
				
				case (ERROR_DELETE) :
					strcpy(ans_client,"0");
					strcpy(buf,"Il n'existe pas de publication avec cet identifiant");
					tc->state = ACCUEIL;
					break;
					
				default :
					break;
			}
			if (strcmp(ans_client,"1") == 0)	//Si on veut que le client réponde ou non
				tc->lec_on = 1;
			else 
				tc->lec_on = 0;
			//printf("On envoie un message %d\n", tc->canal); Message de debug
			ecrireLigne(tc->canal, ans_client);
			ecrireLigne(tc->canal, buf);
			sem_post(&tc->sem_r);
			sem_wait(&tc->sem_w);
		}
	}
	exit(EXIT_SUCCESS);
}

void *commandeServeur(void *arg)	//THREAD SERVEUR 
{
	while (continu)		//VERIFICATION DE DISPONIBILITE D'UN THREAD
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

int main(int argc, char *argv[])
{
	//chargeement	 de la base de données
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
	
	pthread_t id_commandeServeur;
	int serveur_ouvert = 1;
	
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
		
	if (pthread_create(&id_commandeServeur, NULL, commandeServeur, &serveur_ouvert) != 0)
			erreur_IO("creation of the commandServeur Thread");
			

	char buf[BUF_SIZE];
	//static int codeRetour = 0;
	printf("/shutdown pour fermer le serveur\n");
	printf("/save pour sauvegarder\n");
	printf("/data pour afficher les données\n");
	while(continu)	//COMMANDE SERVEUR
	{
		printf(">");
		if(fgets(buf, LIGNE_MAX, stdin) == NULL)
			erreur("fegts null\n");
		if(strcmp(buf, "/shutdown\n") == 0)
		{
			saveDataInFile(data_users.tete_users, &data_users.info);
			printf("Fermeture du serveur ...\n");
			continu = 0;
		}
		else if (strcmp(buf, "/save\n") == 0)
		{
			saveDataInFile(data_users.tete_users, &data_users.info);
		}
		else if (strcmp(buf, "/data\n") == 0)
		{
			printData(data_users.tete_users);
		}
		else
		{
			printf("Commande inconnue, essayer /shutdown ou /save\n");
		}
		
	}
			

	exit(EXIT_SUCCESS);
}


