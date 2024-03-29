#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include "data_function.h"
#include "pse.h"

void *autoSave(void *arg)
{
	DataUtilisateurTete* tete_data_users = (DataUtilisateurTete*) arg;
	while(1)
	{
		sleep(SAVE_DELAY);
		printf("[Sauvegarde Periodique]>Sauvegarde des données...\n");
		saveDataInFile(tete_data_users->tete_users, &tete_data_users->info);
	}
}

DataUtilisateur* findUserById(DataUtilisateur* users, int id_user)
{
	DataUtilisateur* current_user = users;
	while(current_user->utilisateur->id != id_user)
	{
		current_user = current_user->suiv;
	}
	return current_user; 
}

DataUtilisateur* findUserByPseudo(DataUtilisateur* users, char pseudo[])
{
	DataUtilisateur* current_user = users;
	while (current_user != NULL) 
	{
		if (strcmp(pseudo, current_user->utilisateur->pseudo) == 0)
		{
			return current_user;
		}
		current_user = current_user->suiv;
	}
	return NULL;
}

int isMoreRecent(Publication* p1, Publication* p2)
{
	/*Renvoie 1 si p1 est plus recente que p2 0 sinon*/
	if (p1->date->tm_year > p2->date->tm_year)
	{
		return 1;
	}
	else if(p1->date->tm_year == p2->date->tm_year)
	{
		if (p1->date->tm_mon > p2->date->tm_mon)
		{
			return 1;
		}
		else if(p1->date->tm_mon == p2->date->tm_mon)
		{
			if (p1->date->tm_mday > p2->date->tm_mday)
			{
				return 1;
			}
			else if(p1->date->tm_mday == p2->date->tm_mday)
			{
				if (p1->date->tm_hour > p2->date->tm_hour)
				{
					return 1;
				}
				else if(p1->date->tm_hour == p2->date->tm_hour)
				{
					if (p1->date->tm_min > p2->date->tm_min)
					{
						return 1;
					}
					else if(p1->date->tm_min == p2->date->tm_min)
					{
						if (p1->date->tm_sec > p2->date->tm_sec)
						{
							return 1;
						}
						else
						{
							return 0;
						}
					}
					else
					{
						return 0;
					}
				}
				else
				{
					return 0;
				}
			}
			else
			{
				return 0;
			}
		}
		else
		{
			return 0;
		}
	}
	else 
	{
		return 0;
	}

}

void addUtilisateur(DataUtilisateur* users, char pseudo[BUFFER_PSEUDO], char mdp[BUFFER_MDP], DataInfo* info)
{
	//On initialise si c'est le premier utilisateurs
	DataUtilisateur* current_user = users;
	DataUtilisateur* precedent_user = users;
	int id = 0;
	if(current_user->utilisateur != NULL)
	{
		while(current_user != NULL)
		{
			precedent_user = current_user;
			current_user = current_user->suiv;
			id++;
		}
		DataUtilisateur* new_dataUser = (DataUtilisateur*) malloc(sizeof(DataUtilisateur));
		initDataUtilisateur(new_dataUser);
		precedent_user->suiv = new_dataUser;
		Utilisateur* new_user = (Utilisateur*) malloc(sizeof(Utilisateur));
		strcpy(new_user->pseudo, pseudo);
		strcpy(new_user->mdp, mdp);
		new_user->id = precedent_user->utilisateur->id+1;
		new_dataUser->utilisateur = new_user;
	}
	else
	{
		Utilisateur* new_user = (Utilisateur*) malloc(sizeof(Utilisateur));
		strcpy(new_user->pseudo, pseudo);
		strcpy(new_user->mdp, mdp);
		new_user->id = 0;
		users->utilisateur = new_user;
	}
	info->nb_utilisateur++;
}

int deleteUserById(DataUtilisateurTete* data_users, int user_id)
{
	//On initialise si c'est le premier utilisateurs
	DataUtilisateur* current_user = data_users->tete_users;
	DataUtilisateur* precedent_user = data_users->tete_users;
	if(current_user->utilisateur->id == user_id)
	{
		//Si l'utilisateur a delete est le premier
		data_users->tete_users = data_users->tete_users->suiv;
		current_user->suiv = NULL;
		freeUser(current_user);
	}
	else
	{
		while(current_user->utilisateur->id != user_id)
		{
			precedent_user = current_user;
			current_user = current_user->suiv;
			if(current_user == NULL)
			{
				printf("L'utilisateur n'existe pas\n");
				return 1;
			}
		}
		precedent_user->suiv = current_user->suiv;
		current_user->suiv = NULL;
		freeUser(current_user);
	}
	data_users->info.nb_utilisateur--;
	return 0;
}

void addPublication(DataUtilisateur* users, int id_user, char texte[BUFFER_PUBLI], int tm_mday, int tm_mon, int tm_year, int tm_hour, int tm_min, int tm_sec)
{
	DataUtilisateur* current_user = findUserById(users, id_user);
	//On parcours les publications
	Publication* current_publi = current_user->publication;
	Publication* precedent_publi = current_user->publication;
	while(current_publi != NULL)
	{
		precedent_publi = current_publi;
		current_publi = current_publi->suiv;
	}
	Publication* new_publi = (Publication*) malloc(sizeof(Publication));
	strcpy(new_publi->texte, texte);
	new_publi->date =  (struct tm*) malloc(sizeof(struct tm));
	new_publi->date->tm_mday = tm_mday;
	new_publi->date->tm_mon = tm_mon;
	new_publi->date->tm_year  = tm_year;
	new_publi->date->tm_hour = tm_hour;
	new_publi->date->tm_min = tm_min;
	new_publi->date->tm_sec = tm_sec;
	new_publi->suiv = NULL;
	if(current_user->publication != NULL)
	{
		precedent_publi->suiv = new_publi; 
		new_publi->id = precedent_publi->id + 1;
	}
	else
	{
		current_user->publication = new_publi;
		new_publi->id = 0;
	}
	current_user->nb_publication++;
}

void addNewPublication(DataUtilisateur* users, int id_user, char texte[BUFFER_PUBLI])
{
	DataUtilisateur* current_user = findUserById(users, id_user);
	//On parcours les publications
	Publication* precedent_publi = current_user->publication;
	Publication* new_publi = (Publication*) malloc(sizeof(Publication));
	strcpy(new_publi->texte, texte);
	time_t temps;
	time(&temps);
	new_publi->date =  (struct tm*) malloc(sizeof(struct tm));
	new_publi->date = localtime(&temps);
	new_publi->suiv = NULL;
	if(current_user->publication != NULL)
	{
		new_publi->id = precedent_publi->id + 1;
	}
	else
	{
		new_publi->id = 0;
	}
	
	//On place la publication au début de la chaine
	precedent_publi = current_user->publication;
	current_user->publication = new_publi;
	new_publi->suiv = precedent_publi;
	
	current_user->nb_publication++;
}

int deletePublicationById(DataUtilisateur* current_user, int publication_id)
{
	/*Fonction qui supprime la publication d'un utilisateur. Elle prends en paramètre un pointeur vers cette utilisateur et l'id de la publication a supprimer*/
	Publication* current_publi = current_user->publication;
	Publication* precedent_publi = current_user->publication;
	if(current_publi == NULL)
	{
		//Si l'utilisateur n'a aucune publication
		printf("L'utilisateur n'a aucune publication a supprimer\n");
		return 1;
	}
	else if(current_publi->id == publication_id)
	{
		//Si la publication est la première
		current_user->publication = current_user->publication->suiv;
		precedent_publi->suiv = NULL;
		freePublication(precedent_publi);
	}
	else
	{
		//Sinon on parcours les publications
	 	while(current_publi->id != publication_id)
		{	
			precedent_publi = current_publi;
			current_publi = current_publi->suiv;
			if(current_publi == NULL)
			{
				printf("La publication n'existe pas\n");
				return 1;
			}
		}
		precedent_publi->suiv = current_publi->suiv;
		current_publi->suiv = NULL;
		freePublication(current_publi);
	}
	current_user->nb_publication--;
	return 0;
}

int deleteAbonnementByPseudo(DataUtilisateurTete* data_users, DataUtilisateur* current_user, char user_abonnement_pseudo[])
{
	/*Fonction qui supprime l'abonnement d'un utilisateur. Elle prends en paramètre un pointeur vers cette utilisateur et le pseudo de l'utilisateur a ne plus suivre*/
	DataUtilisateur* utilisateur_abo = findUserByPseudo(data_users->tete_users, user_abonnement_pseudo);
	if (utilisateur_abo == NULL)
		return 1; //L'utilisateur n'existe pas;
	UtilisateurChaine* current_abo = current_user->abonnements;
	UtilisateurChaine* precedent_abo = current_user->abonnements;
	//D'abords on doit supprimer l'abonnement de l'utilisateur courant
	if(current_abo == NULL)
	{
		//Si l'utilisateur n'a pas d'abonnement
		printf("L'utilisateur n'a aucune abonnement a supprimer\n");
		return 1;
	}
	else if (strcmp(current_abo->data_user->utilisateur->pseudo,user_abonnement_pseudo) == 0)
	{
		//Si l'abonnement est le premier
		current_user->abonnements = current_user->abonnements->suiv;
		precedent_abo->suiv = NULL;
		freeUtilisateurChaine(precedent_abo);
	}
	else
	{
		//Sinon on parcours les abonnements
	 	while(strcmp(current_abo->data_user->utilisateur->pseudo,user_abonnement_pseudo) != 0)
		{	
			precedent_abo = current_abo;
			current_abo = current_abo->suiv;
			if(current_abo == NULL)
			{
				printf("L'abonnement n'existe pas\n");
				return 1;
			}
		}
		precedent_abo->suiv = current_abo->suiv;
		current_abo->suiv = NULL;
		freeUtilisateurChaine(current_abo);
	}
	current_user->nb_abonnement--;
	//Ensuite on doit supprimer l'abonne de l'utilisateur de l'abonnement
	deleteAbonneByPseudo(data_users, utilisateur_abo, current_user->utilisateur->pseudo);
	return 0;

	
}

int deleteAbonneByPseudo(DataUtilisateurTete* data_users, DataUtilisateur* current_user, char user_abonnement_pseudo[])
{
	/*Fonction qui supprime l'abonne d'un utilisateur. Elle prends en paramètre un pointeur vers cette utilisateur et le pseudo de l'utilisateur a ne plus suivre*/
	UtilisateurChaine* current_abo = current_user->abonnes;
	UtilisateurChaine* precedent_abo = current_user->abonnes;
	//D'abords on doit supprimer l'abonnement de l'utilisateur courant
	if(current_abo == NULL)
	{
		//Si l'utilisateur n'a pas d'abonnement
		printf("L'utilisateur n'a aucune abonnement a supprimer\n");
		return 1;
	}
	else if (strcmp(current_abo->data_user->utilisateur->pseudo,user_abonnement_pseudo) == 0)
	{
		//Si l'abonnement est le premier
		current_user->abonnes = current_user->abonnes->suiv;
		precedent_abo->suiv = NULL;
		freeUtilisateurChaine(precedent_abo);
	}
	else
	{
		//Sinon on parcours les abonnements
	 	while(strcmp(current_abo->data_user->utilisateur->pseudo,user_abonnement_pseudo) == 0)
		{	
			precedent_abo = current_abo;
			current_abo = current_abo->suiv;
			if(current_abo == NULL)
			{
				printf("L'abonne n'existe pas\n");
				return 1;
			}
		}
		precedent_abo->suiv = current_abo->suiv;
		current_abo->suiv = NULL;
		freeUtilisateurChaine(current_abo);
	}
	current_user->nb_abonne--;
	return 0;
}

void addAbonnement(DataUtilisateur* users, int id_user, int id_abonnement)
{
	DataUtilisateur* current_user = findUserById(users, id_user);
	DataUtilisateur* user_abonnement = findUserById(users, id_abonnement);
	
	UtilisateurChaine* current_user_abonnement = current_user->abonnements;
	UtilisateurChaine* precedent_user_abonnement = current_user->abonnements;
	while(current_user_abonnement != NULL)
	{
		precedent_user_abonnement = current_user_abonnement;
		current_user_abonnement = current_user_abonnement->suiv;
	}
	UtilisateurChaine* new_user_chaine = (UtilisateurChaine*) malloc(sizeof(UtilisateurChaine));
	new_user_chaine->data_user = user_abonnement;
	new_user_chaine->suiv = NULL;
	if(current_user->abonnements != NULL)
	{
		precedent_user_abonnement->suiv = new_user_chaine;
	}
	else
	{
		current_user->abonnements = new_user_chaine;
	}
	current_user->nb_abonnement++;
	addAbonne(users, id_abonnement, id_user);
}

int addAbonnementByPseudo(DataUtilisateurTete* data_users, DataUtilisateur* current_user, char pseud_abonnement[])
{
	DataUtilisateur* user_abonnement = findUserByPseudo(data_users->tete_users, pseud_abonnement);
	if(user_abonnement == NULL)
	{
		printf("L'utilisateur n'existe pas");
		return 1;
	}
	if (AbonneToByPseudo(data_users,current_user,user_abonnement) == 0)
		return 2;
	UtilisateurChaine* current_user_abonnement = current_user->abonnements;
	UtilisateurChaine* precedent_user_abonnement = current_user->abonnements;
	while(current_user_abonnement != NULL)
	{
		precedent_user_abonnement = current_user_abonnement;
		current_user_abonnement = current_user_abonnement->suiv;
	}
	UtilisateurChaine* new_user_chaine = (UtilisateurChaine*) malloc(sizeof(UtilisateurChaine));
	new_user_chaine->data_user = user_abonnement;
	new_user_chaine->suiv = NULL;
	if(current_user->abonnements != NULL)
	{
		precedent_user_abonnement->suiv = new_user_chaine;
	}
	else
	{
		current_user->abonnements = new_user_chaine;
	}
	current_user->nb_abonnement++;
	addAbonne(data_users->tete_users, user_abonnement->utilisateur->id, current_user->utilisateur->id);
	return 0;
}

void addAbonne(DataUtilisateur* users, int id_user, int id_abonnee)
{
	DataUtilisateur* current_user = findUserById(users, id_user);
	DataUtilisateur* user_abonnee = findUserById(users, id_abonnee);
	
	UtilisateurChaine* current_user_abonnee = current_user->abonnes;
	UtilisateurChaine* precedent_user_abonnee = current_user->abonnes;
	while(current_user_abonnee != NULL)
	{
		precedent_user_abonnee = current_user_abonnee;
		current_user_abonnee = current_user_abonnee->suiv;
	}
	UtilisateurChaine* new_user_chaine = (UtilisateurChaine*) malloc(sizeof(UtilisateurChaine));
	new_user_chaine->data_user = user_abonnee;
	new_user_chaine->suiv = NULL;
	if(current_user->abonnes != NULL)
	{
		precedent_user_abonnee->suiv = new_user_chaine;
	}
	else
	{
		current_user->abonnes = new_user_chaine;
	}
	current_user->nb_abonne++;
}

int AbonneToByPseudo(DataUtilisateurTete* data_users, DataUtilisateur* current_user, DataUtilisateur* user_abonnement)
{
		UtilisateurChaine* current_user_abonnement = current_user->abonnements;
		while (current_user_abonnement != NULL)
		{
			if (current_user_abonnement->data_user != NULL && current_user_abonnement->data_user == user_abonnement)
				return 0;
			current_user_abonnement = current_user_abonnement->suiv;
		}
		return 1;
}

void printPublicationUser(DataUtilisateur* current_datauser)
{
	Publication* current_publi = current_datauser->publication;
	while(current_publi != NULL)
	{
		printf("	Publication id %d: le: %d/%d/%d ", current_publi->id, current_publi->date->tm_mday, current_publi->date->tm_mon+1, current_publi->date->tm_year+1900);
		printf("a %dh%dmin%ds\n", current_publi->date->tm_hour, current_publi->date->tm_min, current_publi->date->tm_sec);
		printf("		%s\n", current_publi->texte);
		current_publi = current_publi->suiv;
	}
}

void printAbonnementsUser(DataUtilisateur* current_datauser)
{
	UtilisateurChaine* current_abo = current_datauser->abonnements;
	while(current_abo != NULL)
	{
		printf("	Abonnements: %s\n", current_abo->data_user->utilisateur->pseudo);
		current_abo = current_abo->suiv;
	}
}

void printabonnesUser(DataUtilisateur* current_datauser)
{
	UtilisateurChaine* current_abo = current_datauser->abonnes;
	while(current_abo != NULL)
	{
		printf("	Abonnes: %s\n", current_abo->data_user->utilisateur->pseudo);
		current_abo = current_abo->suiv;
	}
}


void printData(DataUtilisateur* users)
{
	DataUtilisateur* current_datauser = users;
	if(users->utilisateur != NULL)
	{
		while(current_datauser != NULL)
		{
			printf("Utilisateur numéro %d: %s\n", current_datauser->utilisateur->id, current_datauser->utilisateur->pseudo);
			printPublicationUser(current_datauser);
			printAbonnementsUser(current_datauser);
			printabonnesUser(current_datauser);
			current_datauser = current_datauser->suiv;
		}
	}
	else
	{
		printf("Utilisateur numéro %d: %s\n", current_datauser->utilisateur->id, current_datauser->utilisateur->pseudo);
	}
	printf("Fin affichage utilisateur\n");
}

void initDataUtilisateur(DataUtilisateur* users)
{
	users->utilisateur = NULL;
	users->publication = NULL;
	users->abonnes = NULL;
	users->abonnements = NULL;
	users->suiv = NULL;
	users->nb_publication = 0;
	users->nb_abonne = 0;
	users->nb_abonnement = 0;
}

int getLastUserId(DataUtilisateurTete* data_users)
{
	DataUtilisateur* current_user = data_users->tete_users;
	while (current_user->suiv != NULL) 
	{
		current_user = current_user->suiv;
	}
	return current_user->utilisateur->id;
}

void loadDataFromFile(DataUtilisateurTete* data_users)
{
	FILE* fichier;
	fichier = fopen("data_1.txt", "r");
	if(fichier != NULL)
	{
		int nb_utilisateur = 0;
		fscanf(fichier, "%d\n", &nb_utilisateur);
		printf("Lecture de %d utilisateurs\n", nb_utilisateur);
		for(int i = 0; i<nb_utilisateur; i++)
		{
			Utilisateur utilisateur;
			fscanf(fichier, "%d %s %s", &utilisateur.id, utilisateur.pseudo, utilisateur.mdp);
			addUtilisateur(data_users->tete_users, utilisateur.pseudo, utilisateur.mdp, &data_users->info);
		}
		
		for(int i = 0; i<nb_utilisateur; i++)
		{
			int id = 0;
			fscanf(fichier, "%d", &id);
			int nb_publication = 0;
			fscanf(fichier, "%d", &nb_publication);
			if(nb_publication > 0)
			{
				Publication publication;
				int tm_mday, tm_mon, tm_year, tm_hour, tm_min, tm_sec;
				for(int j = 0; j<nb_publication; j++)
				{
					fgets(publication.texte,BUFFER_PUBLI, fichier);
					fgets(publication.texte,BUFFER_PUBLI, fichier);
					fscanf(fichier, "%d %d %d %d %d %d", &tm_mday, &tm_mon, &tm_year, &tm_hour, &tm_min, &tm_sec);
					addPublication(data_users->tete_users, id, publication.texte, tm_mday, tm_mon, tm_year, tm_hour, tm_min, tm_sec);
				}
			}
			//Chargement des abonnements
			int nb_abonnement = 0;
			fscanf(fichier, "%d", &nb_abonnement);
			if(nb_abonnement > 0)
			{
				int id_abonnement = 0;
				for(int j = 0; j<nb_abonnement; j++)
				{
					fscanf(fichier, "%d", &id_abonnement);
					addAbonnement(data_users->tete_users, id, id_abonnement);
				}
			}
			//Chargement des abonnes
			int nb_abonne = 0;
			fscanf(fichier, "%d", &nb_abonne);
			if(nb_abonne > 0)
			{
				int id_abonne = 0;
				for(int j = 0; j<nb_abonne; j++)
				{
					fscanf(fichier, "%d", &id_abonne);
					//addAbonne(data_users->tete_users, id, id_abonne);
				}
			}
		}
		fclose(fichier);
	}
	else
	{
		printf("Erreur lors de l'ouverture du fichier de données\n");
	}
}

void saveDataInFile(DataUtilisateur* users, DataInfo* info)
{
	FILE* fichier;
	fichier = fopen("data_1.txt", "w+");
	if(fichier != NULL)
	{
		DataUtilisateur* current_user = users;
		fprintf(fichier, "%d\n", info->nb_utilisateur);
		printf("Enregistrement de %d utilisateurs\n", info->nb_utilisateur);
		for(int i = 0; i<info->nb_utilisateur; i++)
		{
		
			fprintf(fichier, "%d %s %s\n", current_user->utilisateur->id, current_user->utilisateur->pseudo, current_user->utilisateur->mdp);
			//Sauvegarde des publications
			current_user = current_user->suiv;
		}
		current_user = users;
		for(int i = 0; i<info->nb_utilisateur; i++)
		{
			fprintf(fichier, "%d\n", current_user->utilisateur->id);
			fprintf(fichier, "%d\n", current_user->nb_publication);
			if(current_user->nb_publication > 0)
			{
				Publication* current_publi = current_user->publication;
				for(int j = 0; j<current_user->nb_publication; j++)
				{
					fprintf(fichier, "%s", current_publi->texte);
					fprintf(fichier, "%d %d %d ", current_publi->date->tm_mday, current_publi->date->tm_mon, current_publi->date->tm_year);
					fprintf(fichier, "%d %d %d\n", current_publi->date->tm_hour, current_publi->date->tm_min, current_publi->date->tm_sec);
					current_publi = current_publi->suiv;
				}
			}
			//Sauvegarde des abonnements
			fprintf(fichier, "%d\n", current_user->nb_abonnement);
			if(current_user->nb_abonnement > 0)
			{
				UtilisateurChaine* current_abo = current_user->abonnements;
				for(int j = 0; j<current_user->nb_abonnement; j++)
				{
					fprintf(fichier, "%d\n", current_abo->data_user->utilisateur->id);
					current_abo = current_abo->suiv;
				}
			}
			//Sauvegarde des abonnes
			fprintf(fichier, "%d\n", current_user->nb_abonne);
			if(current_user->nb_abonne > 0)
			{
				UtilisateurChaine* current_abo = current_user->abonnes;
				for(int j = 0; j<current_user->nb_abonne; j++)
				{
					fprintf(fichier, "%d\n", current_abo->data_user->utilisateur->id);
					current_abo = current_abo->suiv;
				}
			}
			current_user = current_user->suiv;
		}
		fclose(fichier);
	}
	else
	{
		printf("Erreur lors de l'ouverture du fichier de données\n");
	}
}

void freePublication(Publication* publication)
{
	Publication* current_publi = publication;
	Publication* precedent_publi = publication;
	while(current_publi != NULL)
	{
		precedent_publi = current_publi;
		current_publi = current_publi->suiv;
		free(precedent_publi);
	}
}

void freeUtilisateurChaine(UtilisateurChaine* abonnes)
{
	UtilisateurChaine* current_abo = abonnes;
	UtilisateurChaine* precedent_abo = abonnes;
	while(current_abo != NULL)
	{
		precedent_abo = current_abo;
		current_abo = current_abo->suiv;
		free(precedent_abo);
	}
}

void freeUser(DataUtilisateur* users)
{
	freePublication(users->publication);
	free(users->utilisateur);
	freeUtilisateurChaine(users->abonnements);
	freeUtilisateurChaine(users->abonnes);
	free(users);
}

void freeDataMemory(DataUtilisateur* users)
{
	DataUtilisateur* current_user = users;
	DataUtilisateur* precedent_user = users;
	while(current_user != NULL)
	{
		precedent_user = current_user;
		current_user = current_user->suiv;
		free(precedent_user);
	}
}






