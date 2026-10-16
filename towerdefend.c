#include <stdio.h>
#include <stdlib.h>
#include "SDL.h"
#include "towerdefend.h"
#include "maSDL.h"


/* Allocation verifiee : aucune case du plateau ne reste indeterminee. */
static void *alloue(size_t taille) {
    void *p = calloc(1, taille);
    if (p == NULL) { perror("Allocation memoire"); exit(EXIT_FAILURE); }
    return p;
}

TplateauJeu AlloueTab2D(int largeur, int hauteur) {
    if (largeur <= 0 || hauteur <= 0) return NULL;
    TplateauJeu jeu = alloue((size_t)largeur * sizeof *jeu);
    for (int i = 0; i < largeur; i++)
        jeu[i] = alloue((size_t)hauteur * sizeof *jeu[i]);
    return jeu;
}

void freePlateau(TplateauJeu jeu, int largeur) {
    if (!jeu) return;
    for (int i = 0; i < largeur; i++) free(jeu[i]);
    free(jeu);
}

void initPlateauAvecNULL(TplateauJeu jeu, int largeur, int hauteur) {
    if (!jeu) return;
    for (int i = 0; i < largeur; i++)
        for (int j = 0; j < hauteur; j++) jeu[i][j] = NULL;
}

/* 17 pas vers le haut et 16 pas horizontaux : 34 cases distinctes.
   Chaque ligne n'est visitee qu'une fois, donc aucun croisement n'est possible. */
int **initChemin(void) {
    int **chemin = alloue(NBCOORDPARCOURS * sizeof *chemin);
    for (int i = 0; i < NBCOORDPARCOURS; i++) chemin[i] = alloue(2 * sizeof **chemin);
    int x = LARGEURJEU / 2, y = HAUTEURJEU - 1, index = 0;
    int horizontaux = NBCOORDPARCOURS - (HAUTEURJEU - 1);
    chemin[index][X] = x; chemin[index++][Y] = y;
    while (y > 1) {
        int lignesApres = y - 2;
        int minimum = horizontaux - 3 * lignesApres;
        if (minimum < 0) minimum = 0;
        int maximum = horizontaux < 3 ? horizontaux : 3;
        int pas = minimum + rand() % (maximum - minimum + 1);
        int direction = rand() % 2 ? 1 : -1;
        if (x + direction * pas < 0 || x + direction * pas >= LARGEURJEU) direction = -direction;
        for (int h = 0; h < pas; h++) {
            x += direction;
            chemin[index][X] = x; chemin[index++][Y] = y;
        }
        horizontaux -= pas;
        y--;
        chemin[index][X] = x; chemin[index++][Y] = y;
    }
    return chemin;
}

int cheminValide(int **chemin) {
    if (!chemin) return 0;
    for (int i = 0; i < NBCOORDPARCOURS; i++) {
        if (!chemin[i] || chemin[i][X] < 0 || chemin[i][X] >= LARGEURJEU
            || chemin[i][Y] < 0 || chemin[i][Y] >= HAUTEURJEU) return 0;
        if (i && abs(chemin[i][X] - chemin[i-1][X])
                 + abs(chemin[i][Y] - chemin[i-1][Y]) != 1) return 0;
        for (int j = 0; j < i; j++)
            if (chemin[j][X] == chemin[i][X] && chemin[j][Y] == chemin[i][Y]) return 0;
    }
    return 1;
}

//comanter!!!
void afficheCoordonneesParcours(int **chemin, int nbcoord){
    printf("Liste coordonnees: ");
    for (int i=0; i<nbcoord; i++){
        printf("(%d, %d)",chemin[i][X], chemin[i][Y]);
    }
    printf("\nfin liste coordonnées\n");
}

// Libère la mémoire du chemin
void freeChemin(int **tab){
    if (!tab) return;
    for (int j=0;j<NBCOORDPARCOURS;j++){
        free(tab[j]);
    }
    free(tab);
}

//comanter!!!
void affichePlateauConsole(TplateauJeu jeu, int largeur, int hauteur){

    const char* InitialeUnite[7]={"s", "a", "r", "A", "C", "D", "G"};

    printf("\n");
    for (int j=0;j<hauteur;j++){
        for (int i=0;i<largeur;i++){

            if (jeu[i][j] != NULL){
                    printf("%s",InitialeUnite[jeu[i][j]->nom]);
            }
            else printf(" ");
        }
        printf("\n");
    }
}


/**GESTION DES UNITÉS*/


// Chaque fonction initialise les statistiques spécifiques d'une unité
// Note: Chaque unité possède ses propres PV, dégâts et portée selon l'équilibrage du jeu

Tunite *creeTourSol(int posx, int posy){
    Tunite *nouv = (Tunite*)alloue(sizeof(Tunite));
    nouv->nom = tourSol;
    nouv->cibleAttaquable = sol;
    nouv->maposition = sol;
    nouv->pointsDeVie = 500;
    nouv->vitesseAttaque = 1.5;
    nouv->degats = 120;
    nouv->portee = 5;
    nouv->vitessedeplacement = 0;
    nouv->posX = posx;
    nouv->posY = posy;
    nouv->peutAttaquer = 1;
    return nouv;
}
Tunite *creeTourAir(int posx, int posy){
    Tunite *nouv = (Tunite*)alloue(sizeof(Tunite));
    nouv->nom = tourAir;
    nouv->cibleAttaquable = air;
    nouv->maposition = sol;
    nouv->pointsDeVie = 500;
    nouv->vitesseAttaque = 1.0;
    nouv->degats = 100;
    nouv->portee = 3;
    nouv->vitessedeplacement = 0;
    nouv->posX = posx;
    nouv->posY = posy;
    nouv->peutAttaquer = 1;
    return nouv;
}
Tunite *creeTourRoi(int posx, int posy){
    Tunite *nouv = (Tunite*)alloue(sizeof(Tunite));
    nouv->nom = tourRoi;
    nouv->cibleAttaquable = solEtAir;
    nouv->maposition = sol;
    nouv->pointsDeVie = 800;
    nouv->vitesseAttaque = 1.2;
    nouv->degats = 180;
    nouv->portee = 4;
    nouv->vitessedeplacement = 0;
    nouv->posX = posx;
    nouv->posY = posy;
    nouv->peutAttaquer = 1;
    return nouv;
}
Tunite *creeDragon(int posx, int posy){
    Tunite *nouv = (Tunite*)alloue(sizeof(Tunite));
    nouv->nom = dragon;
    nouv->cibleAttaquable = solEtAir;
    nouv->maposition = air;
    nouv->pointsDeVie = 200;
    nouv->vitesseAttaque = 1.1;
    nouv->degats = 180;
    nouv->portee = 2;
    nouv->vitessedeplacement = 2;
    nouv->posX = posx;
    nouv->posY = posy;
    nouv->peutAttaquer = 1;
    return nouv;
}

Tunite *creeChevalier(int posx, int posy){
    Tunite *nouv = (Tunite*)alloue(sizeof(Tunite));
    nouv->nom = chevalier;
    nouv->cibleAttaquable = sol;
    nouv->maposition = sol;
    nouv->pointsDeVie = 400;
    nouv->vitesseAttaque = 1.5;
    nouv->degats = 250;
    nouv->portee = 1;
    nouv->vitessedeplacement = 2.0;
    nouv->posX = posx;
    nouv->posY = posy;
    nouv->peutAttaquer = 1;
    return nouv;
}

Tunite *creeArcher(int posx, int posy){
    Tunite *nouv = (Tunite*)alloue(sizeof(Tunite));
    nouv->nom = archer;
    nouv->cibleAttaquable = solEtAir;
    nouv->maposition = sol;
    nouv->pointsDeVie = 80;
    nouv->vitesseAttaque = 0.7;
    nouv->degats = 120;
    nouv->portee = 3;
    nouv->vitessedeplacement = 1.0;
    nouv->posX = posx;
    nouv->posY = posy;
    nouv->peutAttaquer = 1;
    return nouv;
}

Tunite *creeGargouille(int posx, int posy){
    Tunite *nouv = (Tunite*)alloue(sizeof(Tunite));
    nouv->nom = gargouille;
    nouv->cibleAttaquable = solEtAir;
    nouv->maposition = air;
    nouv->pointsDeVie = 80;
    nouv->vitesseAttaque = 0.6;
    nouv->degats = 90;
    nouv->portee = 1;
    nouv->vitessedeplacement = 3.0;
    nouv->posX = posx;
    nouv->posY = posy;
    nouv->peutAttaquer = 1;
    return nouv;
}


/**GESTION DES LISTES ET DU PLATEAU*/


// Ajoute une unité en tête de la liste du joueur
void AjouterUnite(TListePlayer *player, Tunite *nouvelleUnite) {
    if (!player || !nouvelleUnite) return;
    TListePlayer nouvelleCellule = (TListePlayer)alloue(sizeof(struct T_cell));
    nouvelleCellule->pdata = nouvelleUnite;
    nouvelleCellule->suiv = *player;
    *player = nouvelleCellule;
}

// Supprime une unité spécifique de la liste et libère sa mémoire
void supprimerUnite(TListePlayer *player, Tunite *UniteDetruite) {
    TListePlayer cour = *player;
    TListePlayer prec = NULL;


    while (cour != NULL && cour->pdata != UniteDetruite) {
        prec = cour;
        cour = cour->suiv;
    }


    if (cour != NULL) {
        if (prec == NULL) {

            *player = cour->suiv;
        } else {

            prec->suiv = cour->suiv;
        }
        free(cour->pdata);
        free(cour);
    }
}

// Remplit le plateau virtuel (tableau 2D) avec les pointeurs des unités des listes
void PositionnePlayerOnPlateau(TListePlayer player, TplateauJeu jeu) {
    TListePlayer cour = player;
    while (cour != NULL) {
        Tunite *u = cour->pdata;

        if (u->posX >= 0 && u->posX < LARGEURJEU && u->posY >= 0 && u->posY < HAUTEURJEU) {
            jeu[u->posX][u->posY] = u;
        }
        cour = cour->suiv;
    }
}


// Vérifie si la Tour du Roi est toujours présente dans la liste du joueur
int tourRoiDetruite(TListePlayer player) {
    TListePlayer cour = player;
    while (cour != NULL) {
        if (cour->pdata->nom == tourRoi) {
            return 0;   // Roi vivant
        }
        cour = cour->suiv;
    }
    return 1;   // Roi mort
}


/**LOGIQUE DE COMBAT ET CIBLAGE*/


// Insère une cible dans une liste temporaire triée par points de vie (priorité au plus faible)
void insererTrie(TListePlayer *liste, Tunite *unite) {
    TListePlayer nouveau = (TListePlayer)alloue(sizeof(struct T_cell));
    nouveau->pdata = unite;
    nouveau->suiv = NULL;


    if (*liste == NULL || (*liste)->pdata->pointsDeVie >= unite->pointsDeVie) {
        nouveau->suiv = *liste;
        *liste = nouveau;
    } else {

        TListePlayer cour = *liste;
        while (cour->suiv != NULL && cour->suiv->pdata->pointsDeVie < unite->pointsDeVie) {
            cour = cour->suiv;
        }
        nouveau->suiv = cour->suiv;
        cour->suiv = nouveau;
    }
}

// Analyse le voisinage de l'unité et retourne la liste des ennemis à portée
TListePlayer quiEstAPortee(TplateauJeu jeu, Tunite *UniteAttaquante) {
    TListePlayer ciblesPotentielles = NULL;
    int portee = UniteAttaquante->portee;

    // Délimitation d'un carré de recherche pour optimiser les performances
    int xMin = (UniteAttaquante->posX - portee >= 0) ? UniteAttaquante->posX - portee : 0;
    int xMax = (UniteAttaquante->posX + portee < LARGEURJEU) ? UniteAttaquante->posX + portee : LARGEURJEU - 1;
    int yMin = (UniteAttaquante->posY - portee >= 0) ? UniteAttaquante->posY - portee : 0;
    int yMax = (UniteAttaquante->posY + portee < HAUTEURJEU) ? UniteAttaquante->posY + portee : HAUTEURJEU - 1;

    for (int i = xMin; i <= xMax; i++) {
        for (int j = yMin; j <= yMax; j++) {
            Tunite *cible = jeu[i][j];


            if (cible != NULL && cible != UniteAttaquante && cible->pointsDeVie > 0) {
                // Identification de l'allégeance
                int estHorde = (UniteAttaquante->nom == archer || UniteAttaquante->nom == chevalier ||
                                UniteAttaquante->nom == dragon || UniteAttaquante->nom == gargouille);

                int estCibleValide = 0;

                // Application des règles
                if (estHorde && cible->nom == tourRoi) {
                    estCibleValide = 1;
                }

                else if (!estHorde && (cible->nom == archer || cible->nom == chevalier ||
                                       cible->nom == dragon || cible->nom == gargouille)) {
                    estCibleValide = 1;
                }

                if (estCibleValide) {
                    // Vérification de la compatibilité Air/Sol
                    int cibleCompatible = 0;
                    if (UniteAttaquante->cibleAttaquable == solEtAir) {
                        cibleCompatible = 1;
                    } else if (UniteAttaquante->cibleAttaquable == cible->maposition) {
                        cibleCompatible = 1;
                    }

                    if (cibleCompatible) {
                        // Calcul de la distance réelle (Euclidienne au carré pour éviter sqrt)
                        int dx = i - UniteAttaquante->posX;
                        int dy = j - UniteAttaquante->posY;
                        if ((dx * dx + dy * dy) <= (portee * portee)) {

                            insererTrie(&ciblesPotentielles, cible);
                        }
                    }
                }
            }
        }
    }
    return ciblesPotentielles;
}

// Réalisation de l'attaque : animation + dégâts
void combat(SDL_Surface *surface, Tunite *UniteAttaquante, Tunite *UniteCible) {

    if (UniteAttaquante != NULL && UniteCible != NULL && UniteAttaquante->pointsDeVie > 0 && UniteCible->pointsDeVie > 0) {


        if (UniteAttaquante->peutAttaquer == 1) {


            if (surface) dessineAttaque(surface, UniteAttaquante, UniteCible);


            UniteCible->pointsDeVie -= UniteAttaquante->degats;


            UniteAttaquante->peutAttaquer = 0;  // L'unité a consommé son tour
        }
    }
}
/**IA ET DÉPLACEMENTS*/


// Vérifie si une case (x,y) appartient au chemin de la horde
int estSurLeChemin(int x, int y, int **chemin) {
    for (int i = 0; i < NBCOORDPARCOURS; i++) {
        if (chemin[i][X] == x && chemin[i][Y] == y) {
            return 1;
        }
    }
    return 0;
}

// Algorithme de placement stratégique des tours en fonction de la proximité du chemin
void placerToursAutomatiquement(TListePlayer *listeRoi, int **chemin, int nbToursAPlacer) {

    int scores[LARGEURJEU][HAUTEURJEU] = {0};


    for (int x = 0; x < LARGEURJEU; x++) {
        for (int y = 0; y < HAUTEURJEU; y++) {


            if (estSurLeChemin(x, y, chemin) || y <= 1) {
                scores[x][y] = -1;
            } else {

                int score = 0;
                for (int i = 0; i < NBCOORDPARCOURS; i++) {
                    int dx = chemin[i][X] - x;
                    int dy = chemin[i][Y] - y;
                    if ((dx * dx + dy * dy) <= (3 * 3)) {
                        score++;
                    }
                }
                scores[x][y] = score;
            }
        }
    }


    for (int t = 0; t < nbToursAPlacer; t++) {
        int meilleurScore = -1;
        int bestX = -1, bestY = -1;


        for (int x = 0; x < LARGEURJEU; x++) {
            for (int y = 0; y < HAUTEURJEU; y++) {
                if (scores[x][y] > meilleurScore) {
                    meilleurScore = scores[x][y];
                    bestX = x;
                    bestY = y;
                }
            }
        }


        if (bestX != -1 && bestY != -1) {

            if (t % 2 == 0) {
                AjouterUnite(listeRoi, creeTourSol(bestX, bestY));
            } else {
                AjouterUnite(listeRoi, creeTourAir(bestX, bestY));
            }

            scores[bestX][bestY] = -1;
        }
    }
}

// Fait avancer les monstres le long des indices du tableau chemin
void deplacerHorde(TListePlayer listeHorde, int **chemin, TplateauJeu jeu) {
    TListePlayer cour = listeHorde;
    while (cour != NULL) {
        Tunite *monstre = cour->pdata;


        int indexActuel = -1;
        for (int i = 0; i < NBCOORDPARCOURS; i++) {
            if (chemin[i][X] == monstre->posX && chemin[i][Y] == monstre->posY) {
                indexActuel = i;
                break;
            }
        }


        if (indexActuel != -1 && indexActuel < NBCOORDPARCOURS - 1) {

            int casesAAvancer = (int)monstre->vitessedeplacement;
            int cibleIndex = indexActuel;


            for(int pas = 1; pas <= casesAAvancer; pas++) {
                if (indexActuel + pas < NBCOORDPARCOURS) {
                    int nextX = chemin[indexActuel + pas][X];
                    int nextY = chemin[indexActuel + pas][Y];


                    if (jeu[nextX][nextY] == NULL) {
                        cibleIndex = indexActuel + pas;
                    } else {

                        break;
                    }
                }
            }


            if (cibleIndex > indexActuel) {
                int nextX = chemin[cibleIndex][X];
                int nextY = chemin[cibleIndex][Y];


                jeu[monstre->posX][monstre->posY] = NULL;
                jeu[nextX][nextY] = monstre;


                monstre->posX = nextX;
                monstre->posY = nextY;
            }
        }
        cour = cour->suiv;
    }
}

// Gère l'apparition aléatoire de nouveaux monstres à l'entrée du chemin
void gererApparitions(TListePlayer *listeHorde, int **chemin) {

    for (TListePlayer u = *listeHorde; u; u = u->suiv)
        if (u->pdata->posX == chemin[0][X] && u->pdata->posY == chemin[0][Y]) return;
    if (rand() % 100 < 30) {
        int xDbt = chemin[0][X];
        int yDbt = chemin[0][Y];


        int type = rand() % 4;
        Tunite *nouveau = NULL;

        if (type == 0) nouveau = creeChevalier(xDbt, yDbt);
        else if (type == 1) nouveau = creeDragon(xDbt, yDbt);
        else if (type == 2) nouveau = creeArcher(xDbt, yDbt);
        else nouveau = creeGargouille(xDbt, yDbt);

        AjouterUnite(listeHorde, nouveau);
    }
}


/**SAUVEGARDES ET CHARGEMENTS*/


// Nettoyage de liste (Libère maillons + données)
void viderListe(TListePlayer *liste) {
    TListePlayer cour = *liste;
    while (cour != NULL) {
        TListePlayer temp = cour;
        cour = cour->suiv;
        free(temp->pdata);
        free(temp);
    }
    *liste = NULL;
}

// Nettoyage de liste temporaire (Libère maillons uniquement)
void viderListeTemporaire(TListePlayer *liste) {
    TListePlayer cour = *liste;
    while (cour != NULL) {
        TListePlayer temp = cour;
        cour = cour->suiv;
        free(temp);
    }
    *liste = NULL;
}


/* Un tour complet reconstruit le plateau apres chaque suppression ou chargement. */
void avancerPartie(TListePlayer *roi, TListePlayer *horde, int **chemin,
                   TplateauJeu jeu, SDL_Surface *surface) {
    if (tourRoiDetruite(*roi)) return;
    gererApparitions(horde, chemin);
    initPlateauAvecNULL(jeu, LARGEURJEU, HAUTEURJEU);
    PositionnePlayerOnPlateau(*roi, jeu);
    PositionnePlayerOnPlateau(*horde, jeu);
    deplacerHorde(*horde, chemin, jeu);
    TListePlayer armees[2] = {*roi, *horde};
    for (int i = 0; i < 2; i++) {
        for (TListePlayer u = armees[i]; u; u = u->suiv) {
            if (u->pdata->pointsDeVie <= 0) continue;
            u->pdata->peutAttaquer = 1;
            TListePlayer cibles = quiEstAPortee(jeu, u->pdata);
            if (cibles) combat(surface, u->pdata, cibles->pdata);
            viderListeTemporaire(&cibles);
        }
    }
    TListePlayer *listes[2] = {roi, horde};
    for (int i = 0; i < 2; i++) {
        TListePlayer u = *listes[i];
        while (u) {
            TListePlayer suivant = u->suiv;
            if (u->pdata->pointsDeVie <= 0) supprimerUnite(listes[i], u->pdata);
            u = suivant;
        }
    }
    initPlateauAvecNULL(jeu, LARGEURJEU, HAUTEURJEU);
    PositionnePlayerOnPlateau(*roi, jeu);
    PositionnePlayerOnPlateau(*horde, jeu);
}

/* Format versionne. Le format binaire reste propre a l'architecture C utilisee. */
#include <math.h>
#include <string.h>

static int uniteValide(const Tunite *u, int faction) {
    return u && u->nom >= tourSol && u->nom <= gargouille
        && (faction == 0 ? u->nom <= tourRoi : u->nom >= archer)
        && u->cibleAttaquable >= sol && u->cibleAttaquable <= air
        && u->maposition >= sol && u->maposition <= air
        && u->posX >= 0 && u->posX < LARGEURJEU && u->posY >= 0 && u->posY < HAUTEURJEU
        && u->pointsDeVie > 0 && u->pointsDeVie <= 10000
        && u->degats >= 0 && u->degats <= 10000 && u->portee >= 0 && u->portee <= HAUTEURJEU
        && isfinite(u->vitesseAttaque) && u->vitesseAttaque > 0 && u->vitesseAttaque <= 10
        && isfinite(u->vitessedeplacement) && u->vitessedeplacement >= 0 && u->vitessedeplacement <= 10
        && (u->peutAttaquer == 0 || u->peutAttaquer == 1);
}

static int etatValide(int **chemin, TListePlayer roi, TListePlayer horde) {
    if (!cheminValide(chemin)) return 0;
    int occupe[LARGEURJEU][HAUTEURJEU] = {{0}}, rois = 0;
    TListePlayer listes[2] = {roi, horde};
    for (int faction = 0; faction < 2; faction++) {
        int count = 0;
        for (TListePlayer c = listes[faction]; c; c = c->suiv) {
            const Tunite *u = c->pdata;
            if (++count > LARGEURJEU * HAUTEURJEU || !uniteValide(u, faction)) return 0;
            if (occupe[u->posX][u->posY]++) return 0;
            if (u->nom == tourRoi) {
                rois++;
                if (u->posX != chemin[NBCOORDPARCOURS-1][X]
                    || u->posY != chemin[NBCOORDPARCOURS-1][Y]) return 0;
            } else if ((faction == 1) != estSurLeChemin(u->posX, u->posY, chemin)) return 0;
        }
    }
    return rois == 1;
}

int sauvegarderPartie(const char *nom, int binaire, int **chemin,
                     TListePlayer roi, TListePlayer horde) {
    if (!nom || !etatValide(chemin, roi, horde)) return 0;
    FILE *f = fopen(nom, binaire ? "wb" : "w");
    if (!f) return 0;
    int ok = 1;
    if (binaire) ok = fwrite("TDEF0001", 1, 8, f) == 8;
    else ok = fprintf(f, "TDEF-TEXT 1\n") > 0;
    for (int i = 0; i < NBCOORDPARCOURS && ok; i++) {
        if (binaire) ok = fwrite(chemin[i], sizeof(int), 2, f) == 2;
        else ok = fprintf(f, "%d %d\n", chemin[i][X], chemin[i][Y]) > 0;
    }
    TListePlayer listes[2] = {roi, horde};
    for (int i = 0; i < 2 && ok; i++) {
        int count = 0;
        for (TListePlayer c = listes[i]; c; c = c->suiv) count++;
        if (binaire) ok = fwrite(&count, sizeof count, 1, f) == 1;
        else ok = fprintf(f, "%d\n", count) > 0;
        for (TListePlayer c = listes[i]; c && ok; c = c->suiv) {
            Tunite *u = c->pdata;
            if (binaire) ok = fwrite(u, sizeof *u, 1, f) == 1;
            else ok = fprintf(f, "%d %d %d %d %.9g %d %d %.9g %d %d %d\n",
                u->nom, u->cibleAttaquable, u->maposition, u->pointsDeVie,
                (double)u->vitesseAttaque, u->degats, u->portee, (double)u->vitessedeplacement,
                u->posX, u->posY, u->peutAttaquer) > 0;
        }
    }
    if (ferror(f)) ok = 0;
    if (fclose(f) != 0) ok = 0;
    return ok;
}

int chargerPartie(const char *nom, int binaire, int **chemin,
                 TListePlayer *roi, TListePlayer *horde) {
    if (!nom || !chemin || !roi || !horde) return 0;
    FILE *f = fopen(nom, binaire ? "rb" : "r");
    if (!f) return 0;
    char magic[16] = {0};
    int version = 0, ok;
    if (binaire) ok = fread(magic, 1, 8, f) == 8 && memcmp(magic, "TDEF0001", 8) == 0;
    else ok = fscanf(f, "%15s %d", magic, &version) == 2
              && strcmp(magic, "TDEF-TEXT") == 0 && version == 1;
    int coordonnees[NBCOORDPARCOURS][2], *nouveauChemin[NBCOORDPARCOURS];
    for (int i = 0; i < NBCOORDPARCOURS; i++) {
        nouveauChemin[i] = coordonnees[i];
        if (ok) {
            if (binaire) ok = fread(coordonnees[i], sizeof(int), 2, f) == 2;
            else ok = fscanf(f, "%d %d", &coordonnees[i][X], &coordonnees[i][Y]) == 2;
        }
    }
    TListePlayer listes[2] = {NULL, NULL};
    for (int faction = 0; faction < 2 && ok; faction++) {
        int count = 0;
        if (binaire) ok = fread(&count, sizeof count, 1, f) == 1;
        else ok = fscanf(f, "%d", &count) == 1;
        if (count < 0 || count > LARGEURJEU * HAUTEURJEU) ok = 0;
        TListePlayer *fin = &listes[faction];
        for (int i = 0; i < count && ok; i++) {
            Tunite *u = alloue(sizeof *u);
            if (binaire) ok = fread(u, sizeof *u, 1, f) == 1;
            else {
                int nomUnite = 0, cible = 0, position = 0;
                ok = fscanf(f, "%d %d %d %d %f %d %d %f %d %d %d",
                    &nomUnite, &cible, &position, &u->pointsDeVie, &u->vitesseAttaque,
                    &u->degats, &u->portee, &u->vitessedeplacement, &u->posX,
                    &u->posY, &u->peutAttaquer) == 11;
                u->nom = (TuniteDuJeu)nomUnite; u->cibleAttaquable = (Tcible)cible;
                u->maposition = (Tcible)position;
            }
            if (!ok || !uniteValide(u, faction)) {free(u); ok = 0; break;}
            *fin = alloue(sizeof **fin); (*fin)->pdata = u; fin = &(*fin)->suiv;
        }
    }
    if (ok) {
        int c;
        do {c = fgetc(f);} while (c == ' ' || c == '\n' || c == '\r' || c == '\t');
        ok = c == EOF && !ferror(f) && etatValide(nouveauChemin, listes[0], listes[1]);
    }
    fclose(f);
    if (!ok) {viderListe(&listes[0]); viderListe(&listes[1]); return 0;}
    /* Ne remplacer la partie courante qu'apres validation complete. */
    viderListe(roi); viderListe(horde);
    *roi = listes[0]; *horde = listes[1];
    for (int i = 0; i < NBCOORDPARCOURS; i++) {
        chemin[i][X] = coordonnees[i][X]; chemin[i][Y] = coordonnees[i][Y];
    }
    return 1;
}

