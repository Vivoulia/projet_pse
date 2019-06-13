#include <time.h>

#define BUFFER_PSEUDO 50
#define BUFFER_MDP 50
#define BUFFER_PUBLI 140
#define SAVE_DELAY 60

typedef struct Publication Publication;
typedef struct Date Date;
typedef struct Utilisateur Utilisateur;
typedef struct UtilisateurChaine UtilisateurChaine;
typedef struct DataUtilisateur DataUtilisateur;
typedef struct DataInfo DataInfo;
typedef struct DataUtilisateurTete DataUtilisateurTete;

void *autoSave(void *arg);

DataUtilisateur* findUserById(DataUtilisateur* users, int id_user);
DataUtilisateur* findUserByPseudo(DataUtilisateur* users, char pseudo[]);
void initDataUtilisateur(DataUtilisateur* users);

void addUtilisateur(DataUtilisateur* users, char pseudo[BUFFER_PSEUDO], char mdp[BUFFER_MDP], DataInfo* info);
void addPublication(DataUtilisateur* users, int id_user, char texte[BUFFER_PUBLI], int tm_mday, int tm_mon, int tm_year, int tm_hour, int tm_min, int tm_sec);
void addNewPublication(DataUtilisateur* users, int id_user, char texte[BUFFER_PUBLI]);
void addAbonnement(DataUtilisateur* users, int id_user, int id_abonnement);
int addAbonnementByPseudo(DataUtilisateurTete* data_users, DataUtilisateur* current_user, char pseud_abonnement[]);
void addAbonne(DataUtilisateur* users, int id_user, int id_abonnee);

int deleteUserById(DataUtilisateurTete* data_users, int user_id);
int deletePublicationById(DataUtilisateur* current_user, int publication_id);
int deleteAbonnementByPseudo(DataUtilisateurTete* data_users, DataUtilisateur* current_user, char user_abonnement_pseudo[]);
int deleteAbonneByPseudo(DataUtilisateurTete* data_users, DataUtilisateur* current_user, char user_abonnement_pseudo[]);

void printPublicationUser(DataUtilisateur* current_datauser);
void printAbonnementsUser(DataUtilisateur* current_datauser);
void printabonnesUser(DataUtilisateur* current_datauser);
void printData(DataUtilisateur* users);
void loadDataFromFile(DataUtilisateurTete* data_users);
void saveDataInFile(DataUtilisateur* users, DataInfo* info);
void freeDataMemory(DataUtilisateur* users);

void freeUser(DataUtilisateur* users);
void freeUtilisateurChaine(UtilisateurChaine* abonnes);
void freePublication(Publication* publication);

int getLastUserId(DataUtilisateurTete* data_users);

struct Date
{
	int jour;
	int mois;
	int annee;
};

struct Publication
{
	char texte[BUFFER_PUBLI];
	struct tm* date;
	Publication* suiv;
	int id;
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
	Utilisateur* utilisateur;
	Publication* publication;
	UtilisateurChaine* abonnes;
	UtilisateurChaine* abonnements;
	DataUtilisateur* suiv;
	int nb_publication;
	int nb_abonne;
	int nb_abonnement;
};

struct DataInfo
{
	int nb_utilisateur;
	int nb_publication;
	pthread_t id_thread_save;
};

struct DataUtilisateurTete
{
	DataUtilisateur* tete_users;
	DataInfo info;
};



