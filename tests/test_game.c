#include "towerdefend.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned long controles = 0;
#define VERIFIER(c) do { controles++; if (!(c)) { fprintf(stderr, "ECHEC ligne %d : %s\n", __LINE__, #c); exit(EXIT_FAILURE); } } while (0)

static int listesEgales(TListePlayer a, TListePlayer b) {
    while (a && b) {
        if (memcmp(a->pdata, b->pdata, sizeof(Tunite)) != 0) return 0;
        a = a->suiv; b = b->suiv;
    }
    return !a && !b;
}

static void invariants(TListePlayer roi, TListePlayer horde, TplateauJeu jeu) {
    int occupe[LARGEURJEU][HAUTEURJEU] = {{0}};
    TListePlayer listes[2] = {roi, horde};
    for (int faction = 0; faction < 2; faction++)
        for (TListePlayer c = listes[faction]; c; c = c->suiv) {
            Tunite *u = c->pdata;
            VERIFIER(u && u->pointsDeVie > 0);
            VERIFIER(u->posX >= 0 && u->posX < LARGEURJEU && u->posY >= 0 && u->posY < HAUTEURJEU);
            VERIFIER(!occupe[u->posX][u->posY]++);
            VERIFIER(jeu[u->posX][u->posY] == u);
        }
    for (int x = 0; x < LARGEURJEU; x++)
        for (int y = 0; y < HAUTEURJEU; y++) VERIFIER((jeu[x][y] != NULL) == (occupe[x][y] != 0));
}

static void testerCibles(void) {
    TplateauJeu jeu = AlloueTab2D(LARGEURJEU, HAUTEURJEU);
    Tunite *tour = creeTourSol(5,5), *faible = creeArcher(5,6), *fort = creeChevalier(6,5);
    Tunite *volant = creeDragon(4,5), *amie = creeTourAir(5,4), *loin = creeArcher(0,0);
    jeu[5][5] = tour; jeu[5][6] = faible; jeu[6][5] = fort;
    jeu[4][5] = volant; jeu[5][4] = amie; jeu[0][0] = loin;
    TListePlayer cibles = quiEstAPortee(jeu, tour);
    VERIFIER(cibles && cibles->pdata == faible && cibles->suiv && cibles->suiv->pdata == fort && !cibles->suiv->suiv);
    viderListeTemporaire(&cibles);
    int pv = fort->pointsDeVie;
    combat(NULL, tour, fort); VERIFIER(fort->pointsDeVie == pv-tour->degats);
    combat(NULL, tour, fort); VERIFIER(fort->pointsDeVie == pv-tour->degats);
    tour->pointsDeVie = 0; tour->peutAttaquer = 1;
    combat(NULL, tour, fort); VERIFIER(fort->pointsDeVie == pv-tour->degats);
    faible->pointsDeVie = 0; cibles = quiEstAPortee(jeu, amie);
    VERIFIER(cibles && cibles->pdata == volant && !cibles->suiv);
    viderListeTemporaire(&cibles);
    free(tour); free(faible); free(fort); free(volant); free(amie); free(loin);
    freePlateau(jeu, LARGEURJEU);
}

static void testerSauvegardes(void) {
    srand(42);
    int **chemin = initChemin(), **autre = initChemin();
    TListePlayer roi = NULL, horde = NULL, copieRoi = NULL, copieHorde = NULL;
    AjouterUnite(&roi, creeTourRoi(chemin[33][X], chemin[33][Y]));
    placerToursAutomatiquement(&roi, chemin, 5);
    AjouterUnite(&horde, creeChevalier(chemin[0][X], chemin[0][Y]));
    AjouterUnite(&horde, creeDragon(chemin[1][X], chemin[1][Y]));
    horde->pdata->pointsDeVie = 123;
    for (int binaire = 0; binaire < 2; binaire++) {
        const char *nom = binaire ? "test-partie.tdb" : "test-partie.tds";
        VERIFIER(sauvegarderPartie(nom, binaire, chemin, roi, horde));
        VERIFIER(chargerPartie(nom, binaire, autre, &copieRoi, &copieHorde));
        VERIFIER(listesEgales(roi, copieRoi) && listesEgales(horde, copieHorde));
        for (int i = 0; i < NBCOORDPARCOURS; i++) VERIFIER(memcmp(chemin[i], autre[i], 2*sizeof(int)) == 0);
        TListePlayer ancien = copieRoi;
        FILE *f = fopen("test-corrompu", "wb"); VERIFIER(f != NULL);
        fputs(binaire ? "TDEF0001" : "TDEF-TEXT 1\n0", f); fclose(f);
        VERIFIER(!chargerPartie("test-corrompu", binaire, autre, &copieRoi, &copieHorde));
        VERIFIER(copieRoi == ancien && listesEgales(roi, copieRoi) && listesEgales(horde, copieHorde));
        if (binaire) {
            int invalide = 999999;
            f = fopen(nom, "r+b"); VERIFIER(f != NULL);
            VERIFIER(fseek(f, 8 + NBCOORDPARCOURS*2*(long)sizeof(int), SEEK_SET) == 0);
            VERIFIER(fwrite(&invalide,sizeof invalide,1,f) == 1); fclose(f);
            VERIFIER(!chargerPartie(nom, 1, autre, &copieRoi, &copieHorde));
            VERIFIER(copieRoi == ancien);
        } else {
            f = fopen(nom,"a"); VERIFIER(f != NULL); fputs("INATTENDU",f); fclose(f);
            VERIFIER(!chargerPartie(nom,0,autre,&copieRoi,&copieHorde));
            VERIFIER(copieRoi == ancien);
        }
        remove(nom);
    }
    VERIFIER(!chargerPartie("fichier-inexistant.tdb", 1, autre, &copieRoi, &copieHorde));
    VERIFIER(!sauvegarderPartie("dossier-inexistant/test.tdb", 1, chemin, roi, horde));
    remove("test-corrompu");
    viderListe(&roi); viderListe(&horde); viderListe(&copieRoi); viderListe(&copieHorde);
    freeChemin(chemin); freeChemin(autre);
}

int main(void) {
    for (unsigned int seed = 0; seed < 500; seed++) {
        srand(seed); int **chemin = initChemin();
        VERIFIER(cheminValide(chemin));
        VERIFIER(chemin[0][Y] == HAUTEURJEU-1 && chemin[33][Y] == 1);
        TplateauJeu jeu = AlloueTab2D(LARGEURJEU, HAUTEURJEU);
        TListePlayer roi = NULL;
        AjouterUnite(&roi, creeTourRoi(chemin[33][X],chemin[33][Y]));
        placerToursAutomatiquement(&roi, chemin, 5);
        PositionnePlayerOnPlateau(roi, jeu); invariants(roi, NULL, jeu);
        int count = 0; for(TListePlayer u=roi;u;u=u->suiv) count++;
        VERIFIER(count == 6);
        viderListe(&roi); freePlateau(jeu,LARGEURJEU); freeChemin(chemin);
    }
    testerCibles(); testerSauvegardes();
    for(unsigned int seed=0; seed<40; seed++) {
        srand(seed); int **chemin=initChemin();
        TplateauJeu jeu=AlloueTab2D(LARGEURJEU,HAUTEURJEU);
        TListePlayer roi=NULL,horde=NULL;
        AjouterUnite(&roi,creeTourRoi(chemin[33][X],chemin[33][Y]));
        placerToursAutomatiquement(&roi,chemin,5);
        for(int tour=0;tour<300 && !tourRoiDetruite(roi);tour++) {
            avancerPartie(&roi,&horde,chemin,jeu,NULL); invariants(roi,horde,jeu);
        }
        viderListe(&roi); viderListe(&horde); freePlateau(jeu,LARGEURJEU); freeChemin(chemin);
    }
    printf("OK : 500 parcours, 40 simulations, ciblage, combat et sauvegardes ; %lu controles.\n",controles);
    return EXIT_SUCCESS;
}
