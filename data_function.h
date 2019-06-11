#define BUFFER_PSEUDO 50
#define BUFFER_MDP 50
#define BUFFER_PUBLI 140
#define SAVE_DELAY 300

typedef struct Publication Publication;
typedef struct Date Date;
typedef struct Utilisateur Utilisateur;
typedef struct UtilisateurChaine UtilisateurChaine;
typedef struct DataUtilisateur DataUtilisateur;
typedef struct DataInfo DataInfo;
typedef struct DataUtilisateurTete DataUtilisateurTete;

void *autoSave(void *arg);

DataUtilisateur* finUserById(DataUtilisateur* users, int id_user);
DataUtilisateur* findUserByPseudo(DataUtilisateur* users, char pseudo[]);
void initDataUtilisateur(DataUtilisateur* users);

void addUtilisateur(DataUtilisateur* users, char pseudo[BUFFER_PSEUDO], char mdp[BUFFER_MDP], DataInfo* info);
void addPublication(DataUtilisateur* users, int id_user, char texte[BUFFER_PUBLI]);
void addAbonnement(DataUtilisateur* users, int id_user, int id_abonnement);
void addAbonne(DataUtilisateur* users, int id_user, int id_abonnee);

int deleteUserById(DataUtilisateurTete* data_users, int user_id);
void deletePublicationById(DataUtilisateurTete* data_users, int publication_id);

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
	Date date;
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



