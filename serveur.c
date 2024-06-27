#include <stdio.h>
#include <winsock2.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <process.h>
#include <ws2tcpip.h>


typedef struct {
    char nom_utilisateur[20];
    int mot_de_passe;
} Connexion;

typedef struct {
    char rue[100];
    char ville[50];
    char pays[50];
} Adresse;

typedef struct {
    char nom[50];
    char prenom[50];
    char gsm[30];
    char email[50];
    Adresse adresse;
} Contact;

// Structure pour stocker les informations du client
typedef struct {
    SOCKET clientSocket;
    struct sockaddr_in clientAddr;
    char nom_utilisateur[20];
} ClientInfo;

void ajouterContact(SOCKET client, Contact nouveauContact) {
    // Traiter les informations du contact reçues
    printf("Nouveau contact ajouté :\n");
    printf("Nom : %s\n", nouveauContact.nom);
    printf("Prenom : %s\n", nouveauContact.prenom);
    printf("GSM : %s\n", nouveauContact.gsm);
    printf("Email : %s\n", nouveauContact.email);
    printf("Adresse :\n");
    printf("  Rue : %s\n", nouveauContact.adresse.rue);
    printf("  Ville : %s\n", nouveauContact.adresse.ville);
    printf("  Pays : %s\n", nouveauContact.adresse.pays);

    // Ouvrir le fichier contacts.txt en mode ajout
    FILE* f;
    f = fopen("contacts.txt", "a");
    if (f != NULL) {
        fprintf(f, "%s %s %s %s %s %s %s\n", nouveauContact.nom, nouveauContact.prenom, nouveauContact.gsm, nouveauContact.email, nouveauContact.adresse.rue, nouveauContact.adresse.ville, nouveauContact.adresse.pays);
        fclose(f);
        // Envoi de la confirmation de l'ajout au client
        char confirmation[] = "Contact ajouté avec succès.";
        send(client, confirmation, strlen(confirmation) + 1, 0);
    }
    else {
        // En cas d'échec de l'ouverture du fichier
        char erreur[] = "Erreur : fichier introuvable!!";
        send(client, erreur, sizeof(erreur), 0);
    }
}

void rechercher(SOCKET client, char* email) {
    Contact c; // Variable pour stocker temporairement les informations du contact trouvé
    // Ouverture du fichier de contacts en lecture
    FILE* f = fopen("contacts.txt", "r");
    if (f == NULL) {
        printf("Erreur lors de l'ouverture du fichier de contacts.\n");
        char buffer[] = "Erreur lors de l'ouverture du fichier de contacts.";
        send(client, buffer, strlen(buffer) + 1, 0);
        return;
    }

    // Recherche du contact correspondant à l'email
    int found = 0;
    while (fscanf(f, "%s %s %s %s %s %s %s", c.nom, c.prenom, c.gsm, c.email, c.adresse.rue, c.adresse.ville, c.adresse.pays) != EOF) {
        if (strcmp(email, c.email) == 0) {
            found = 1;
            break;
        }
    }

    // Fermeture du fichier
    fclose(f);

    // Envoi du résultat de la recherche au client
    if (found) {
        // Envoi de la structure contact
        send(client, (char*)&c, sizeof(Contact), 0);
    }
    else {
        // Si le contact n'est pas trouvé, envoyer une structure contact vide avec un email vide
        strcpy(c.email, "");
        send(client, (char*)&c, sizeof(Contact), 0);
    }
}

void supprimerContact(SOCKET client, char* email) {
    // Ouvrir le fichier de contacts en mode lecture et écriture
    FILE* f = fopen("contacts.txt", "r+");
    if (f == NULL) {
        printf("Erreur lors de l'ouverture du fichier de contacts.\n");
        char buffer[] = "Erreur lors de l'ouverture du fichier de contacts.";
        send(client, buffer, strlen(buffer) + 1, 0);
        return;
    }

    // Créer un fichier temporaire pour stocker les contacts sans celui à supprimer
    FILE* temp = fopen("temp.txt", "w");
    if (temp == NULL) {
        printf("Erreur lors de la création du fichier temporaire.\n");
        char buffer[] = "Erreur lors de la création du fichier temporaire.";
        send(client, buffer, strlen(buffer) + 1, 0);
        fclose(f);
        return;
    }

    Contact c;
    int found = 0;
    // Parcourir le fichier de contacts
    while (fscanf(f, "%s %s %s %s %s %s %s", c.nom, c.prenom, c.gsm, c.email, c.adresse.rue, c.adresse.ville, c.adresse.pays) != EOF) {
        // Vérifier si l'email correspond au contact à supprimer
        if (strcmp(email, c.email) == 0) {
            found = 1;
            // Ne pas écrire ce contact dans le fichier temporaire (c'est comme le supprimer)
        }
        else {
            // Écrire les autres contacts dans le fichier temporaire
            fprintf(temp, "%s %s %s %s %s %s %s\n", c.nom, c.prenom, c.gsm, c.email, c.adresse.rue, c.adresse.ville, c.adresse.pays);
        }
    }

    // Fermer les fichiers
    fclose(f);
    fclose(temp);

    // Remplacer le fichier original par le fichier temporaire
    if (remove("contacts.txt") != 0 || rename("temp.txt", "contacts.txt") != 0) {
        printf("Erreur lors de la suppression du contact.\n");
        char buffer[] = "Erreur lors de la suppression du contact.";
        send(client, buffer, strlen(buffer) + 1, 0);
        return;
    }

    // Envoyer une confirmation au client
    if (found) {
        char buffer[] = "Contact supprimé avec succès.";
        send(client, buffer, strlen(buffer) + 1, 0);
    }
    else {
        char buffer[] = "Le contact n'a pas été trouvé.";
        send(client, buffer, strlen(buffer) + 1, 0);
    }
}

void Modifier(SOCKET client) {
    char email[30]; // Email du contact à modifier
    Contact nouveau_contact; // Nouvelles informations du contact
    char reponse[100]; // Réponse à envoyer au client
    FILE *f, *temp;
    Contact c;

    // Réception de l'email du client
    if (recv(client, email, sizeof(email), 0) == SOCKET_ERROR) {
        printf("Erreur lors de la réception de l'email de modification.\n");
        strcpy(reponse, "Erreur lors de la réception de l'email de modification.");
        send(client, reponse, sizeof(reponse), 0);
        return;
    }

    // Réception des nouvelles informations du client
    if (recv(client, (char*)&nouveau_contact, sizeof(Contact), 0) == SOCKET_ERROR) {
        printf("Erreur lors de la réception des nouvelles informations du contact.\n");
        strcpy(reponse, "Erreur lors de la réception des nouvelles informations du contact.");
        send(client, reponse, sizeof(reponse), 0);
        return;
    }

    // Ouverture du fichier original en lecture
    f = fopen("contacts.txt", "r");
    if (f == NULL) {
        printf("Erreur lors de l'ouverture du fichier de contacts.\n");
        strcpy(reponse, "Erreur lors de l'ouverture du fichier de contacts.");
        send(client, reponse, sizeof(reponse), 0);
        return;
    }

    // Ouverture d'un fichier temporaire en écriture
    temp = fopen("temp.txt", "w");
    if (temp == NULL) {
        printf("Erreur lors de l'ouverture du fichier temporaire.\n");
        strcpy(reponse, "Erreur lors de l'ouverture du fichier temporaire.");
        send(client, reponse, sizeof(reponse), 0);
        fclose(f);
        return;
    }

    // Lecture du fichier original ligne par ligne
    while (fscanf(f, "%s %s %s %s %s %s %s", c.nom, c.prenom, c.gsm, c.email, c.adresse.rue, c.adresse.ville, c.adresse.pays) != EOF) {
        // Si l'email du contact correspond à celui à modifier, écrire les nouvelles informations dans le fichier temporaire
        if (strcmp(email, c.email) == 0) {
            fprintf(temp, "%s %s %s %s %s %s %s\n", nouveau_contact.nom, nouveau_contact.prenom, nouveau_contact.gsm, nouveau_contact.email, nouveau_contact.adresse.rue, nouveau_contact.adresse.ville, nouveau_contact.adresse.pays);
        } else {
            // Sinon, écrire le contact sans modification
            fprintf(temp, "%s %s %s %s %s %s %s\n", c.nom, c.prenom, c.gsm, c.email, c.adresse.rue, c.adresse.ville, c.adresse.pays);
        }
    }

    // Fermeture des fichiers
    fclose(f);
    fclose(temp);

    // Suppression du fichier original
    remove("contacts.txt");
    // Renommage du fichier temporaire en fichier original
    rename("temp.txt", "contacts.txt");

    // Envoi de la réponse au client
    strcpy(reponse, "Modification effectuée avec succès.");
    send(client, reponse, sizeof(reponse), 0);
}

void AfficherTous(SOCKET client) {
    FILE *f;
    char buffer[1024]; // Buffer pour stocker temporairement les informations des contacts
    Contact c;

    // Ouverture du fichier de contacts en lecture
    f = fopen("contacts.txt", "r");
    if (f == NULL) {
        printf("Erreur lors de l'ouverture du fichier de contacts.\n");
        strcpy(buffer, "Erreur lors de l'ouverture du fichier de contacts.");
        send(client, buffer, sizeof(buffer), 0);
        return;
    }

    // Lecture du fichier de contacts et envoi des informations au client
    while (fscanf(f, "%s %s %s %s %s %s %s", c.nom, c.prenom, c.gsm, c.email, c.adresse.rue, c.adresse.ville, c.adresse.pays) != EOF) {
        sprintf(buffer, "%s %s %s %s %s %s %s\n", c.nom, c.prenom, c.gsm, c.email, c.adresse.rue, c.adresse.ville, c.adresse.pays);
        send(client, buffer, sizeof(buffer), 0);
    }

    // Envoi d'un message de fin au client
    strcpy(buffer, "Fin");
    send(client, buffer, sizeof(buffer), 0);

    // Fermeture du fichier
    fclose(f);
}

#define PORT 50000
#define MAX_TENTATIVES_CONNEXION 3

void handleClient(void *arg) {
    ClientInfo *clientInfo = (ClientInfo *)arg;
    SOCKET clientSocket = clientInfo->clientSocket;
    struct sockaddr_in clientAddr = clientInfo->clientAddr;

    // Obtention de l'adresse IP du client
    char* clientIP = inet_ntoa(clientAddr.sin_addr);
    if (clientIP == NULL) {
        printf("Erreur lors de la récupération de l'adresse IP du client.\n");
        closesocket(clientSocket);
        return;
    }

    // Authentifier l'utilisateur
    Connexion connexionRecue,connexionStockee;
    int typeUtilisateur = 0;
    int choix;
    int tentativesConnexion = 0; // compteur de tentatives
    Contact nouveauContact; 
    char email[30]; // Email du contact à rechercher
    char emailASupprimer[50];
    
    bool deconnexionDemandee = false; // Variable pour indiquer si le client a demandé la déconnexion

    // Boucle pour l'authentification de l'utilisateur
    do {
        // Recevoir les informations d'authentification du client
        if (recv(clientSocket, (char *)&connexionRecue, sizeof(Connexion), 0) == SOCKET_ERROR) {
            printf("Erreur lors de la réception des informations d'authentification du client.\n");
            closesocket(clientSocket);
            return;
        }
        FILE *fichierAuthentification;
        // Ouvrir le fichier d'authentification
        fichierAuthentification = fopen("authentification.txt", "r");
        if (fichierAuthentification == NULL) {
            printf("Échec d'ouverture du fichier d'authentification.\n");
            closesocket(clientSocket);
            return;
        }

        // Comparer l'authentification reçue avec les authentifications stockées
        char typeUtilisateurString[20]; // Stocker le type d'utilisateur en tant que chaîne de caractères
        int trouve = 0;
        while (fscanf(fichierAuthentification, "%s %d %s", connexionStockee.nom_utilisateur, &connexionStockee.mot_de_passe, typeUtilisateurString) == 3) {
            if (strcmp(connexionRecue.nom_utilisateur, connexionStockee.nom_utilisateur) == 0 && connexionRecue.mot_de_passe == connexionStockee.mot_de_passe) {
                // Si l'authentification correspond, définir le typeUtilisateur et indiquer qu'une correspondance est trouvée
                trouve = 1;
                break;
            }
        }

        // Fermer le fichier d'authentification
        fclose(fichierAuthentification);

        // Envoyer une réponse au client
        if (trouve) {
            if (strcmp(typeUtilisateurString, "admin") == 0) {
                // Si l'utilisateur est un administrateur, envoyer 1 au client
                typeUtilisateur = 1;
            } else if (strcmp(typeUtilisateurString, "invite") == 0) {
                // Si l'utilisateur est un invité, envoyer 2 au client
                typeUtilisateur = 2;
            }
            send(clientSocket, (char *)&typeUtilisateur, sizeof(int), 0);

            // Si l'authentification est réussie, sortir de la boucle
            break;
        } else {
            // Si aucune authentification correspondante n'est trouvée, envoyer 0 au client
            typeUtilisateur = 0;
            printf("Nom d'utilisateur ou mot de passe invalide.\n");
     
	 
	        send(clientSocket, (char *)&typeUtilisateur, sizeof(int), 0);

            // Incrémenter les tentatives de connexion
            tentativesConnexion++;
        }
    } while (tentativesConnexion < MAX_TENTATIVES_CONNEXION) ;

    // Si le nombre maximal de tentatives de connexion est atteint, terminer la connexion
    if (tentativesConnexion >= (MAX_TENTATIVES_CONNEXION)) {
        printf("Nombre maximal de tentatives de connexion atteint. Fermeture de la connexion.\n");
        typeUtilisateur = -1;
        send(clientSocket, (char *)&typeUtilisateur, sizeof(int), 0);
        closesocket(clientSocket);
        return;
    }

    printf("Utilisateur %s authentifié depuis l'adresse IP %s.\n", connexionRecue.nom_utilisateur, clientIP);


    // Boucle pour gérer les demandes des clients
    while (!deconnexionDemandee) {
        // Recevoir le choix de l'utilisateur
        int bytesReceived = recv(clientSocket, (char *)&choix, sizeof(int), 0);
        if (bytesReceived == SOCKET_ERROR || bytesReceived == 0) {
            // S'il y a une erreur de réception ou si le client se déconnecte
            if (bytesReceived == 0) {
                printf("Le client s'est déconnecté.\n");
            } else {
                printf("Erreur lors de la réception du choix de l'utilisateur.\n");
            }
            break; // Sortir de la boucle si une erreur de réception se produit ou si le client se déconnecte
        }
        
        // Traiter la demande en fonction du choix de l'utilisateur et de son type
        switch (typeUtilisateur) {
            case 1: // Administrateur
                switch (choix) {
                    case 1:
                        // Recevoir les données du contact à ajouter
                        if (recv(clientSocket, (char*)&nouveauContact, sizeof(Contact), 0) == SOCKET_ERROR) {
                            printf("Erreur lors de la réception des données du contact à ajouter.\n");
                            break;
                        }
                        printf("%s : Demande d'ajout de contact reçue .\n", connexionRecue.nom_utilisateur);
                        ajouterContact(clientSocket, nouveauContact);
                        break;
                    case 2:
                        // Réception de l'email du client
                        if (recv(clientSocket, email, sizeof(email), 0) == SOCKET_ERROR) {
                            printf("Erreur lors de la réception de l'email de recherche.\n");
                            char buffer[] = "Erreur lors de la réception de l'email de recherche.";
                            send(clientSocket, buffer, strlen(buffer) + 1, 0);
                            break;
                        }
                        printf("%s : Demande de recherche de contact reçue  .\n", connexionRecue.nom_utilisateur);
                        rechercher(clientSocket,email);
                        break;
                    case 3:
                        // Supprimer un contact
                        // Recevoir l'email du contact à supprimer du client
                        if (recv(clientSocket, emailASupprimer, sizeof(emailASupprimer), 0) == SOCKET_ERROR) {
                            printf("Erreur lors de la réception de l'email de suppression du client.\n");
                            break;
                        }
                        printf("%s : Demande de suppression de contact reçue .\n", connexionRecue.nom_utilisateur);
                        // Appeler une fonction pour supprimer le contact
                        supprimerContact(clientSocket, emailASupprimer);
                        break;
                    case 4:
                        printf("%s : Demande de modification de contact reçue .\n", connexionRecue.nom_utilisateur);
                        Modifier(clientSocket);
                        break;
                    case 5:
                        printf("%s : Demande d'afficher tous les contacts reçue .\n", connexionRecue.nom_utilisateur);
                        AfficherTous(clientSocket);
                        break;
                    case 6:
                        // Déconnecter le client et sortir de la boucle
                        printf("%s : Déconnexion demandée. Fermeture de la connexion.\n", connexionRecue.nom_utilisateur);
                        closesocket(clientSocket);
                        deconnexionDemandee = true;
                        break;
                    default:
                        printf("%s : Commande invalide reçue .\n", connexionRecue.nom_utilisateur);
                        break;
                }
                break;
            case 2: // Utilisateur régulier
                switch (choix) {
                    case 1:
                        // Réception de l'email du client
                        if (recv(clientSocket, email, sizeof(email), 0) == SOCKET_ERROR) {
                            printf("Erreur lors de la réception de l'email de recherche.\n");
                            char buffer[] = "Erreur lors de la réception de l'email de recherche.";
                            send(clientSocket, buffer, strlen(buffer) + 1, 0);
                            break;
                        }
                        // Appeler la fonction de recherche pour les utilisateurs
                        printf("%s : Demande de recherche de contact reçue .\n", connexionRecue.nom_utilisateur);
                        rechercher(clientSocket,email);
                        break;
                    case 2:
                        printf("%s : Demande d'afficher tous les contacts reçue .\n", connexionRecue.nom_utilisateur);
                        AfficherTous(clientSocket);
                        break;
                    case 3:
                        // Déconnecter le client et sortir de la boucle
                        printf("%s : Déconnexion demandée. Fermeture de la connexion.\n", connexionRecue.nom_utilisateur);
                        closesocket(clientSocket);
                        deconnexionDemandee = true;
                        break;
                    default:
                        printf("%s : Commande invalide reçue .\n", connexionRecue.nom_utilisateur);
                        break;
                }
                break;
            default:
                printf("Utilisateur non authentifié.\n");
                break;
        }
    }

    // Fermer le socket du client
    closesocket(clientSocket);

}


int main() {
    WSADATA wsa;
    SOCKET serveur, client;
    struct sockaddr_in addrServeur, addrClient;
    int tailleAddrClient = sizeof(addrClient);

    // Initialiser Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Échec de l'initialisation de Winsock.\n");
        return 1;
    }

    // Créer le socket
    if ((serveur = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Échec de création du socket.\n");
        WSACleanup();
        return 1;
    }

    // Adresse du serveur
    memset(&addrServeur, 0, sizeof(addrServeur));
    addrServeur.sin_family = AF_INET;
    addrServeur.sin_addr.s_addr = INADDR_ANY;
    addrServeur.sin_port = htons(PORT);

    // Lier le socket
    if (bind(serveur, (struct sockaddr*)&addrServeur, sizeof(addrServeur)) == SOCKET_ERROR) {
        printf("Échec de liaison du socket.\n");
        closesocket(serveur);
        WSACleanup();
        return 1;
    }

    // Écouter les connexions entrantes
    if (listen(serveur, 5) == SOCKET_ERROR) {
        printf("Échec d'écoute.\n");
        closesocket(serveur);
        WSACleanup();
        return 1;
    }

    printf("Le serveur écoute sur le port %d...\n", PORT);

    // Boucle pour accepter les connexions et gérer chaque client
    while (1) {
        // Accepter la connexion
        if ((client = accept(serveur, (struct sockaddr*)&addrClient, &tailleAddrClient)) == INVALID_SOCKET) {
            printf("Échec d'acceptation de la connexion.\n");
            closesocket(serveur);
            WSACleanup();
            return 1;
        }

        // Créer une structure ClientInfo pour stocker les informations du client
        ClientInfo* clientInfo = (ClientInfo*)malloc(sizeof(ClientInfo));
        clientInfo->clientSocket = client;
        clientInfo->clientAddr = addrClient;

        // Créer un thread pour gérer le client
        HANDLE threadHandle;
        DWORD threadID;
        threadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)handleClient, (LPVOID)clientInfo, 0, &threadID);

        // Vérifier si la création du thread a réussi
        if (threadHandle == NULL) {
            printf("Échec de la création du thread pour gérer le client.\n");
            closesocket(client);
        }
        else {
            // Fermer le handle du thread (le thread se fermera tout seul à la fin)
            CloseHandle(threadHandle);
        }
    }

    // Fermer le socket du serveur et nettoyer
    closesocket(serveur);
    WSACleanup();
    return 0;
}
