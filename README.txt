##	Projet PSE - Réalisé par Otello Gonçalves et Vivien Debuchy	##

- - PRINCIPE DU PROGRAMME - -

1.	Ce programme est une application client / serveur permettant de simuler un petit réseau social similaire a twitter. Les utilisateurs peuvent se creer un compte et publier des textes courts. Les utilisateurs peuvent s'abonner à d'autres utilisateurs pour enrichir leur fil d'actualité générer a partir des publications recentes. Il est aussi possible de consulter le profil des autres utilisateurs pour avoir accès a plus d'information.

2.	Au niveau technique le programme est construit avec une architecture de worker statique. Le nombre de worker maximum peut être changé dans les #define du fichier serveur.c. Chaque worker possède deux threads : 
	Un thread de lecture et un thread d'écriture (le fonctionnement est similaire du côté client). Du côté serveur un thread supplémentaire permet de réaliser des sauvegardes periodiques de la structure de données pour limiter la perte en cas de panne.
	Le programme est construit comme une sorte de machine à état qui réagit à des événements. Il est séparé en 3 fichiers principaux:
		- serveur.c : qui represente le coeur du programme. Tous les états et les events y sont traités ainsi que la connexion des clients et les threads.
		- client.c:  qui est simplement la partie client du programme qui permet de dialoguer avec le serveur
		- data_function.c : qui regroupe toutes les structures et les fonctions permettant de gérer la base de données du site (liste chainée, enregistrement et chargement, libération de la mémoire ect ..)

- - INSTALLATION -- 

1. 	Dézippez l'archive .tar du projet de manière à placer les fichiers dans un dossier du répertoire PSE (contenant la librairie pse).
	Par exemple PSE/repertoire_projet/ (ici repertoire_projet contient directement les fichiers sources). 
	
2.	À l'aide du terminal placez vous dans le repertoire du projet et tapez la commande "make". Un Makefile va compiler directement le projet.

3.	Attention un fichier data_1.txt est fournit comme exemple pour le projet. Si ce fichier est supprimé un fichier minimal devra être créer a la main pour que le serveur puisse se lancer. Vous trouverez ci-dessous le code minimal
	a renseigner dans le fichier data_1.txt pour lancer le serveur. Ce fichier creer au moins un utilisateur (ici User0 avec un mot de passe test).
	
	1
	0 User0 test
	0
	0
	0
	0
	
- - UTILISATION - -
	
1. 	Dans le terminal lancez le serveur avec la commande ./serveur [port]

2.	Dans le terminal lancez des clients avec la commande ./client [addresse] [port]

3. 	Les informations d'utilisation sont affichèes a l'écran. Dans le cas du serveur si vous ne voyez plus l'information pour entrer une commande appuyez simplement sur Entrée avec votre clavier.


	
	
