#include <stdio.h>
#include <winsock2.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char nom_utilisateur[20];
    int mot_de_passe;
} Connexion;

typedef struct Adresse {
    char rue[100];
    char ville[50];
    char pays[50];
} adresse;

typedef struct {
    char nom[50];
    char prenom[50];
    char gsm[30];
    char email[50];
    struct Adresse adresse;
} Contact;

void ajouterContact(SOCKET client) {
    Contact nouveauContact;

    // Demander les informations du contact à l'utilisateur
    printf("Nom du contact : ");
    scanf("%s", nouveauContact.nom);
    printf("Prénom du contact : ");
    scanf("%s", nouveauContact.prenom);
    printf("GSM du contact : ");
    scanf("%s", nouveauContact.gsm);
    printf("Email du contact : ");
    scanf("%s", nouveauContact.email);
    printf("Adresse - Nom de la rue : ");
    scanf("%s", nouveauContact.adresse.rue);
    printf("Adresse - Ville : ");
    scanf("%s", nouveauContact.adresse.ville);
    printf("Adresse - Pays : ");
    scanf("%s", nouveauContact.adresse.pays);
    printf("Contact saisi : #%s#%s#%s#%s#%s#%s#%s\n", nouveauContact.nom, nouveauContact.prenom, nouveauContact.gsm, nouveauContact.email, nouveauContact.adresse.rue, nouveauContact.adresse.ville, nouveauContact.adresse.pays);

    // Envoyer les informations du contact au serveur
    send(client, (char *)&nouveauContact, sizeof(Contact), 0);

    // Attendre la réponse du serveur
    char reponse[100];
    recv(client, reponse, sizeof(reponse), 0);

    // Afficher la réponse du serveur
    printf("Réponse du serveur : %s\n", reponse);
}

void rechercherContact(SOCKET server) {
    char email[30]; // Email du contact à rechercher

    // Saisir l'email du contact à rechercher
    printf("Veuillez saisir l'email du contact que vous voulez rechercher : ");
    scanf("%s", email);

    // Envoyer l'email au serveur
    if (send(server, email, sizeof(email), 0) == SOCKET_ERROR) {
        printf("Erreur lors de l'envoi de l'email au serveur.\n");
        return;
    }

    // Attendre la réponse du serveur
    Contact c;
    if (recv(server, (char*)&c, sizeof(Contact), 0) == SOCKET_ERROR) {
        printf("Erreur lors de la réception des données du contact.\n");
        return;
    }

    // Vérifier si l'email a été trouvé
    if (strlen(c.email) == 0) {
        printf("Le contact avec l'email '%s' n'a pas été trouvé.\n", email);
    } else {
        printf("Contact trouvé :\n");
        printf("Nom : %s\n", c.nom);
        printf("Prénom : %s\n", c.prenom);
        printf("GSM : %s\n", c.gsm);
        printf("Email : %s\n", c.email);
        printf("Adresse :\n");
        printf("  Rue : %s\n", c.adresse.rue);
        printf("  Ville : %s\n", c.adresse.ville);
        printf("  Pays : %s\n", c.adresse.pays);
    }
}

void supprimerContact(SOCKET server) {

    char emailASupprimer[50];
    printf("Entrez l'email du contact à supprimer : ");
    scanf("%s", emailASupprimer);
    // Envoyer l'email du contact à supprimer au serveur
    if (send(server, emailASupprimer, sizeof(emailASupprimer), 0) == SOCKET_ERROR) {
        printf("Erreur lors de l'envoi de la demande de suppression de contact.\n");
        return;
    }

    // Attendre la réponse du serveur
    char response[100];
    if (recv(server, response, sizeof(response), 0) == SOCKET_ERROR) {
        printf("Erreur lors de la réception de la réponse du serveur.\n");
        return;
    }

    // Afficher la réponse du serveur
    printf("Réponse du serveur : %s\n", response);
}


void Modifier(SOCKET client) {
    char email[30]; 
    Contact nouveau_contact; 
    char reponse[100]; 

    printf("Entrez l'email du contact à modifier : ");
    scanf("%s", email);

    // Demandez les nouvelles informations du contact
    printf("**Saisie des nouvelles informations du contact**\n");
    printf("Nom du contact : ");
    scanf("%s", nouveau_contact.nom);
    printf("Prenom du contact : ");
    scanf("%s", nouveau_contact.prenom);
    printf("GSM du contact : ");
    scanf("%s", nouveau_contact.gsm);
    printf("Email du contact : ");
    scanf("%s", nouveau_contact.email);
    printf("Adresse - Nom de la rue : ");
    scanf("%s", nouveau_contact.adresse.rue);
    printf("Adresse - Ville : ");
    scanf("%s", nouveau_contact.adresse.ville);
    printf("Adresse - Pays : ");
    scanf("%s", nouveau_contact.adresse.pays);

    // Envoyer l'email au serveur pour la modification
    send(client, email, strlen(email) + 1, 0);
    // Envoyer les nouvelles informations au serveur
    send(client, (char*)&nouveau_contact, sizeof(Contact), 0);

    // Réception de la réponse du serveur
    recv(client, reponse, sizeof(reponse), 0);

    // Affichage de la réponse du serveur
    printf("Réponse du serveur : %s\n", reponse);
}

void AfficherTousContacts(SOCKET client) {
    char buffer[1024]; // Buffer pour recevoir les informations des contacts

    // Réception des informations des contacts depuis le serveur
    while (1) {
        // Réception des informations d'un contact
        if (recv(client, buffer, sizeof(buffer), 0) == SOCKET_ERROR) {
            printf("Erreur lors de la réception des contacts.\n");
            break;
        }
        // Vérification de la fin des données
        if (strcmp(buffer, "Fin") == 0) {
            break;
        }
        // Affichage des informations du contact
        printf("%s", buffer);
    }
}

void menuAdmin(SOCKET client) {
    int choix;
    char email[50]; // Déclarer la variable email ici

    do {
        printf("\n*******Menu*******\n");
        printf("1-Ajouter un contact\n");
        printf("2-Rechercher un contact\n");
        printf("3-Supprimer un contact\n");
        printf("4-Modifier un contact\n");
        printf("5-Afficher tous les contacts\n");
        printf("6-Quitter\n");
        printf("Entrez votre choix : ");
        scanf("%d", &choix);

        // Envoyer le choix au serveur
        send(client, (char *)&choix, sizeof(int), 0);

        // Traiter la réponse du serveur si nécessaire
        switch (choix) {
            case 1:
                // Ajouter un contact
                ajouterContact(client);
                break;
            case 2 :
                rechercherContact(client);
                break;
            case 3:
                // Appeler la fonction pour supprimer le contact
                supprimerContact(client);
                break;
            case 4:
                // Appeler la fonction de modification de contact
                Modifier(client); 
                break;
            case 5:
                // Appeler la fonction d'affichage de tous les contacts
                printf("********Liste des Contacts******** \n");
                AfficherTousContacts(client);
                break;
            case 6:
                // Quitter
                printf("Bye");
                break;
            default:
                printf("Choix invalide.\n");
                break;
        }
    } while (choix != 6);
}

void menuUtilisateur(SOCKET client) {
    int choix;
    do {
        printf("\n*******Menu*******\n");
        printf("1-Rechercher un contact\n");
        printf("2-Afficher tous les contacts\n");
        printf("3-Quitter\n");
        printf("Entrez votre choix : ");
        scanf("%d", &choix);
        // Envoyer le choix au serveur
        send(client, (char *)&choix, sizeof(int), 0);
        // Traiter la réponse du serveur si nécessaire
        switch (choix) {
            case 1:
                // Appeler la fonction de recherche pour les utilisateurs
                rechercherContact(client);
                break;
            case 2:
                // Appeler la fonction d'affichage de tous les contacts
                printf("********Liste des Contacts******** \n");
                AfficherTousContacts(client);
                break;
             
            case 3:
                // Quitter
                printf("Vous avez quitté");
                break;
            default:
                printf("Choix invalide.\n");
                break;
        }
    } while (choix != 3);
}


#define PORT 50000
#define MAX_TENTATIVES_LOGIN 3

int main() {
    WSADATA wsa;
    SOCKET client, server;
    struct sockaddr_in serverAddr;
    Connexion infoLogin;
    int typeUtilisateur;
    int tentatives = 0; // compteur de tentatives

    // Initialisation de Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Échec de l'initialisation de Winsock.\n");
        return 1;
    }

    // Créer la socket
    if ((client = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Échec de création de la socket.\n");
        WSACleanup();
        return 1;
    }

    // Adresse du serveur
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("100.104.161.205");

    // Se connecter au serveur
    if (connect(client, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("Échec de la connexion au serveur.\n");
        closesocket(client);
        WSACleanup();
        return 1;
    }
    printf("Connexion établie.\n");

    do {
        // Saisir les informations de connexion
        printf("Entrez votre nom d'utilisateur : ");
        scanf("%s", infoLogin.nom_utilisateur);
        printf("Entrez votre mot de passe : ");
        scanf("%d", &infoLogin.mot_de_passe);

        // Envoyer les informations de connexion au serveur
        send(client, (char *)&infoLogin, sizeof(Connexion), 0);

        // Recevoir le type d'utilisateur du serveur
        recv(client, (char *)&typeUtilisateur, sizeof(int), 0);

        // Vérifier le type d'utilisateur et afficher le message approprié
        switch (typeUtilisateur) {
            case 1:
                printf("Vous êtes un administrateur.\n");
                menuAdmin(client);
                break;
            case 2:
                printf("Vous êtes un utilisateur invité.\n");
                 menuUtilisateur(client);
                break;
            case 0:
                printf("Nom d'utilisateur ou mot de passe incorrect. Veuillez réessayer.\n");
                tentatives++;
                break;
            default:
                printf("Réponse inconnue du serveur.\n");
                break;
        }

        // Si le nombre de tentatives dépasse 3, arrêter le programme
        if (tentatives >= MAX_TENTATIVES_LOGIN) {
            printf("Nombre maximum de tentatives de connexion atteint. Sortie.\n");

            return 1;
        }
    } while (typeUtilisateur == 0); // Continuer à boucler jusqu'à ce qu'un utilisateur valide soit connecté

    // Fermer la socket
    closesocket(client);
    WSACleanup();

    return 0;
}
