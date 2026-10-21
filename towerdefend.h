#ifndef TOWERDEFEND_H_INCLUDED
#define TOWERDEFEND_H_INCLUDED

#include <SDL.h>


/**CONSTANTES DU JEU*/


#define LARGEURJEU 11       // Nombre de cases en largeur
#define HAUTEURJEU 19       // Nombre de cases en hauteur
#define NBCOORDPARCOURS 34  // Longueur fixe du chemin de la horde
#define X 0                 // Indice pour l'accès aux coordonnées X
#define Y 1                 // Indice pour l'accès aux coordonnées Y


/**ENUMERATIONS*/


// Types d'unités disponibles dans le jeu
typedef enum{tourSol, tourAir, tourRoi, archer, chevalier, dragon, gargouille} TuniteDuJeu;
// Types de cibles et de positions
typedef enum{sol, solEtAir, air } Tcible;


/**STRUCTURES DE DONNÉES*/


// Structure représentant une unité (Orde ou Tour)
typedef struct {
    TuniteDuJeu nom;            // Identifiant du type d'unité
    Tcible cibleAttaquable;     // Ce que l'unité peut viser
    Tcible maposition;          // Où se trouve l'unité

    int pointsDeVie;            // Santé de l'unité
    float vitesseAttaque;       // Cadence de tir
    int degats;                 // Puissance de chaque attaque
    int portee ;                // Rayon d'action en nombre de cases


    float vitessedeplacement;   // Vitesse de mouvement sur le chemin
    int posX, posY;             // Coordonnées actuelles sur la grille
    int peutAttaquer;           // Drapeau booléen (1: disponible pour attaquer, 0: a déjà attaqué)

} Tunite;

// Liste chaînée dynamique pour gérer les unités d'un joueur
typedef struct T_cell{
    struct T_cell *suiv;    // Pointeur vers le maillon suivant
    Tunite *pdata;          // Pointeur vers les données de l'unité
} *TListePlayer;

// Le plateau est un tableau 2D de pointeurs vers des unités
typedef Tunite* ** TplateauJeu;


/**PROTOTYPES DES FONCTIONS*/


// Gestion de la mémoire et du terrain
TplateauJeu AlloueTab2D(int largeur, int hauteur); // comenter !!!
void afficheCoordonneesParcours(int **t, int nbcoord);
int **initChemin(void);
void freeChemin(int **tab);
void freePlateau(TplateauJeu jeu, int largeur);
int cheminValide(int **chemin);
void initPlateauAvecNULL(TplateauJeu jeu,int largeur, int hauteur);
void affichePlateauConsole(TplateauJeu jeu, int largeur, int hauteur);

// Constructeurs d'unités
Tunite *creeTourSol(int posx, int posy);
Tunite *creeTourAir(int posx, int posy);
Tunite *creeTourRoi(int posx, int posy);
Tunite *creeArcher(int posx, int posy);
Tunite *creeGargouille(int posx, int posy);
Tunite *creeDragon(int posx, int posy);
Tunite *creeChevalier(int posx, int posy);

// Logique de jeu et combat
int tourRoiDetruite(TListePlayer player);
void PositionnePlayerOnPlateau(TListePlayer player, TplateauJeu jeu);
TListePlayer quiEstAPortee(TplateauJeu jeu, Tunite *UniteAttaquante) ;
void combat(SDL_Surface *surface , Tunite * UniteAttaquante, Tunite * UniteCible);
void supprimerUnite(TListePlayer *player, Tunite *UniteDetruite);
void AjouterUnite(TListePlayer *player, Tunite *nouvelleUnite);
void placerToursAutomatiquement(TListePlayer *listeRoi, int **chemin, int nbToursAPlacer);
void deplacerHorde(TListePlayer listeHorde, int **chemin, TplateauJeu jeu);
void gererApparitions(TListePlayer *listeHorde, int **chemin);

// Utilitaires de listes
void viderListe(TListePlayer *liste);
void viderListeTemporaire(TListePlayer *liste);// a comenter !!

// Systèmes de sauvegarde
/* Les sauvegardes versionnees incluent aussi le parcours. */
int sauvegarderPartie(const char *fichier, int binaire, int **chemin,
                     TListePlayer listeRoi, TListePlayer listeHorde);
int chargerPartie(const char *fichier, int binaire, int **chemin,
                 TListePlayer *listeRoi, TListePlayer *listeHorde);
void avancerPartie(TListePlayer *listeRoi, TListePlayer *listeHorde,
                   int **chemin, TplateauJeu jeu, SDL_Surface *surface);

#endif
