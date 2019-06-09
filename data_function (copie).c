#include <stdio.h>
#include <stdlib.h>
#include "string.h"

#define BUFFER_PSEUDO 50
#define BUFFER_MDP 50
#define BUFFER_PUBLI 140

typedef struct Publication Publication;
typedef struct Date Date;
typedef struct Utilisateur Utilisateur;
typedef struct UtilisateurChaine UtilisateurChaine;
typedef struct DataUtilisateur DataUtilisateur;
typedef struct TeteAbonnees TeteAbonnees
typedef struct TeteAbonnenements TeteAbonnenements



struct Date
{
	int jour;
	int mois;
	int annee;
};

struct Publication
{
	char texte[BUFFER_PUBLI];
	Date date;
	Publication* suiv;
};

struct Utilisateur
{
	int id;
	char pseudo[BUFFER_PSEUDO];
	char mdp[BUFFER_MDP];
};

struct UtilisateurChaine
{
	Utilisateur* utilisateur;
	UtilisateurChaine* suiv;
};


struct DataUtilisateur
{
	Utilisateur utilisateur;
	Publication publication;
	UtilisateurChaine abonnees;
	UtilisateurChaine abonnements;
	DataUtilisateur* suiv;
	
};


void addUtilisateur(DataUtilisateur* allData, char pseudo[BUFFER_PSEUDO], char mdp[BUFFER_MDP])
{
	DataUtilisateur* current_user;
	current_user = allData;
	while(current_user->suiv != NULL)
	{
		current_user = current_user->suiv;
	}
	DataUtilisateur* new_user = (DataUtilisateur*) malloc(sizeof(DataUtilisateur));
	current_user->suiv = new_user;
	new_user->utilisateur.id = current_user->utilisateur.id + 1;
	strcpy(new_user->utilisateur.pseudo, pseudo); 
	strcpy(new_user->utilisateur.pseudo, pseudo); 
	new_user->suiv = NULL;
	new_user->publication.suiv = NULL;
	strcpy(new_user->publication.texte, "Bienvenue sur votre nouveau compte");
	new_user->abonnees.utilisateur = NULL;
	new_user->abonnees.suiv = NULL;
	new_user->abonnements.utilisateur = NULL;
	new_user->abonnements.suiv = NULL;
	
}

void addPublication(DataUtilisateur* allData, int id_user, char texte[BUFFER_PUBLI])
{
	DataUtilisateur* current_user;
	current_user = allData;
	while(current_user->utilisateur.id != id_user)
	{
		current_user = current_user->suiv;
	}
	
	Publication* current_publi;
	Publication* precedent_publi;
	current_publi = current_user->publication.suiv;
	precedent_publi = &current_user->publication;
	while(current_publi != NULL)
	{
		precedent_publi = current_publi;
		current_publi = current_publi->suiv;
	}
	Publication* new_publi = (Publication*) malloc(sizeof(Publication)); 
	precedent_publi->suiv = new_publi;
	strcpy(new_publi->texte, texte);
	new_publi->suiv = NULL;
}


void addAbonnement(DataUtilisateur* allData, int id_user, int id_abonnement)
{
	DataUtilisateur* current_user;
	current_user = allData;
	while(current_user->utilisateur.id != id_user)
	{
		current_user = current_user->suiv;
	}
	
	DataUtilisateur* current_user_abonnement;
	current_user_abonnement = allData;
	while(current_user_abonnement->utilisateur.id != id_user)
	{
		current_user_abonnement = current_user_abonnement->suiv;
	}
	
	if (current_user.abonnements.utilisateur == NULL)
	{
		current_user.abonnements.utilisateur = current_user_abonnement;
		current_user.abonnements->suiv = NULL;
	}
	else
	{
		UtilisateurChaine* current_abonnement = current_user.abonnements.utilisateur
		while(current_abonnement->suiv != NULL)
		{
			current_abonnement = current_abonnement->suiv;
		}
		current_abonnement->
		
	}
	

}

void printData(DataUtilisateur* allData)
{
	DataUtilisateur* current_user;
	current_user = allData;
	while(current_user != NULL)
	{
		printf("%s\n", current_user->utilisateur.pseudo);
		Publication* current_publi;
		current_publi = &current_user->publication;
		while(current_publi != NULL)
		{
			printf("%s\n", current_publi->texte);
			current_publi = current_publi->suiv;
		}
		current_user = current_user->suiv;
	}
}
int main()
{
	DataUtilisateur allData;
	Utilisateur admin;
	strcpy(admin.pseudo, "Admin");
	strcpy(admin.mdp, "SiSiLaFamille42");
	admin.id = 0;
	allData.utilisateur = admin;
	allData.publication.suiv = NULL;
	strcpy(allData.publication.texte, "Compte Administrateur");
	allData.suiv = NULL;
	
	addUtilisateur(&allData, "Vivien", "123456");
	addPublication(&allData, 1, "Oh une publication");
	
	printData(&allData);
	
	
	return 0;
}


