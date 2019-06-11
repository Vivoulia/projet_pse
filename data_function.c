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

DataUtilisateur* finUserById(DataUtilisateur* users, int id_user)
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

void addPublication(DataUtilisateur* users, int id_user, char texte[BUFFER_PUBLI])
{
	DataUtilisateur* current_user = finUserById(users, id_user);
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
	new_publi->date.jour = 0;
	new_publi->date.mois = 0;
	new_publi->date.annee = 0;
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

void deletePublicationById(DataUtilisateurTete* data_users, int publication_id)
{
}

void addAbonnement(DataUtilisateur* users, int id_user, int id_abonnement)
{
	DataUtilisateur* current_user = finUserById(users, id_user);
	DataUtilisateur* user_abonnement = finUserById(users, id_abonnement);
	
	UtilisateurChaine* current_user_abonnement = current_user->abonnements;
	UtilisateurChaine* precedent_user_abonnement = current_user->abonnements;
	while(current_user_abonnement != NULL)
	{
		precedent_user_abonnement = current_user_abonnement;
		current_user_abonnement = current_user_abonnement->suiv;
	}
	UtilisateurChaine* new_user_chaine = (UtilisateurChaine*) malloc(sizeof(UtilisateurChaine));
	new_user_chaine->utilisateur = user_abonnement->utilisateur;
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
}

void addAbonne(DataUtilisateur* users, int id_user, int id_abonnee)
{
	DataUtilisateur* current_user = finUserById(users, id_user);
	DataUtilisateur* user_abonnee = finUserById(users, id_abonnee);
	
	UtilisateurChaine* current_user_abonnee = current_user->abonnes;
	UtilisateurChaine* precedent_user_abonnee = current_user->abonnes;
	while(current_user_abonnee != NULL)
	{
		precedent_user_abonnee = current_user_abonnee;
		current_user_abonnee = current_user_abonnee->suiv;
	}
	UtilisateurChaine* new_user_chaine = (UtilisateurChaine*) malloc(sizeof(UtilisateurChaine));
	new_user_chaine->utilisateur = user_abonnee->utilisateur;
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

void printPublicationUser(DataUtilisateur* current_datauser)
{
	Publication* current_publi = current_datauser->publication;
	while(current_publi != NULL)
	{
		printf("	Publication id:%d: %s\n", current_publi->id, current_publi->texte);
		current_publi = current_publi->suiv;
	}
}

void printAbonnementsUser(DataUtilisateur* current_datauser)
{
	UtilisateurChaine* current_abo = current_datauser->abonnements;
	while(current_abo != NULL)
	{
		printf("	Abonnements: %s\n", current_abo->utilisateur->pseudo);
		current_abo = current_abo->suiv;
	}
}

void printabonnesUser(DataUtilisateur* current_datauser)
{
	UtilisateurChaine* current_abo = current_datauser->abonnes;
	while(current_abo != NULL)
	{
		printf("	Abonnes: %s\n", current_abo->utilisateur->pseudo);
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
				for(int j = 0; j<nb_publication; j++)
				{
					fscanf(fichier, "%s", publication.texte);
					fscanf(fichier, "%d %d %d\n", &publication.date.jour, &publication.date.mois, &publication.date.annee);
					addPublication(data_users->tete_users, id, publication.texte);
				}
			}
			//Sauvegarde des abonnements
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
			//Sauvegarde des abonnes
			int nb_abonne = 0;
			fscanf(fichier, "%d", &nb_abonne);
			if(nb_abonne > 0)
			{
				int id_abonne = 0;
				for(int j = 0; j<nb_abonne; j++)
				{
					fscanf(fichier, "%d", &id_abonne);
					addAbonne(data_users->tete_users, id, id_abonne);
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
					fprintf(fichier, "%s\n", current_publi->texte);
					fprintf(fichier, "%d %d %d\n", current_publi->date.jour, current_publi->date.mois, current_publi->date.annee);
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
					fprintf(fichier, "%d\n", current_abo->utilisateur->id);
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
					fprintf(fichier, "%d\n", current_abo->utilisateur->id);
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
	printf("A faire");
}






