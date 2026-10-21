// Projet d'origine : SAID PAUL ; second auteur non renseigne dans les sources.
#include <SDL.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "maSDL.h"
#include "towerdefend.h"

static void reconstruire(TplateauJeu jeu, TListePlayer roi, TListePlayer horde) {
    initPlateauAvecNULL(jeu, LARGEURJEU, HAUTEURJEU);
    PositionnePlayerOnPlateau(roi, jeu);
    PositionnePlayerOnPlateau(horde, jeu);
}

static void nouvellePartie(TListePlayer *roi, TListePlayer *horde, int ***chemin,
                          TplateauJeu jeu) {
    viderListe(roi); viderListe(horde); freeChemin(*chemin);
    *chemin = initChemin();
    AjouterUnite(roi, creeTourRoi((*chemin)[NBCOORDPARCOURS-1][X], (*chemin)[NBCOORDPARCOURS-1][Y]));
    placerToursAutomatiquement(roi, *chemin, 5);
    AjouterUnite(horde, creeChevalier((*chemin)[0][X], (*chemin)[0][Y]));
    reconstruire(jeu, *roi, *horde);
}

static void titre(SDL_Window *fenetre, TListePlayer roi, int pause, const char *etat) {
    int pv = 0;
    for (TListePlayer u = roi; u; u = u->suiv)
        if (u->pdata->nom == tourRoi) pv = u->pdata->pointsDeVie;
    char texte[256];
    SDL_snprintf(texte, sizeof texte, "Tower Defense | Roi : %d PV | %s | %s", pv,
                 pv <= 0 ? "Defaite - N pour rejouer" : (pause ? "Pause" : "En cours"), etat);
    SDL_SetWindowTitle(fenetre, texte);
}

static int cheminFichier(char *sortie, size_t taille, const char *dossier, const char *nom) {
    int n = SDL_snprintf(sortie, taille, "%s/%s", dossier, nom);
    return n >= 0 && (size_t)n < taille;
}

int main(int argc, char **argv) {
    int smoke = 0, resultat = EXIT_FAILURE;
    const char *snapshot = NULL, *dossier = NULL;
    unsigned int seed = (unsigned int)time(NULL);
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--smoke-test") == 0 && i + 1 < argc) {
            char *fin; errno = 0;
            long n = strtol(argv[++i], &fin, 10);
            if (errno || *fin || n < 1 || n > 10000) return EXIT_FAILURE;
            smoke = (int)n;
        } else if (strcmp(argv[i], "--snapshot") == 0 && i + 1 < argc) snapshot = argv[++i];
        else if (strcmp(argv[i], "--save-dir") == 0 && i + 1 < argc) dossier = argv[++i];
        else if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            char *fin; errno = 0;
            unsigned long n = strtoul(argv[++i], &fin, 10);
            if (errno || *fin || n > 0xffffffffUL) return EXIT_FAILURE;
            seed = (unsigned int)n;
        } else {
            fprintf(stderr, "Options : --seed N --save-dir DOSSIER --smoke-test N --snapshot IMAGE.bmp\n");
            return EXIT_FAILURE;
        }
    }
    SDL_SetMainReady();
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        fprintf(stderr, "Initialisation SDL impossible : %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }
    SDL_Window *fenetre = SDL_CreateWindow("Tower Defense", SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, LARGEURJEU * 40, HAUTEURJEU * 40,
        smoke ? SDL_WINDOW_HIDDEN : SDL_WINDOW_SHOWN);
    SDL_Surface *sprites[11] = {NULL};
    SDL_Surface *ecran = NULL;
    TplateauJeu jeu = NULL;
    TListePlayer roi = NULL, horde = NULL;
    int **chemin = NULL;
    char *base = NULL, *preferences = NULL;
    if (!fenetre || !(ecran = SDL_GetWindowSurface(fenetre))) goto fin;
    base = SDL_GetBasePath();
    const char *noms[11] = {"TourSol", "TourAir", "TourRoi", "Archer", "Chevalier",
                            "Dragon", "Gargouille", "Eau", "Herbe", "Pont", "Terre"};
    for (int i = 0; i < 11; i++) {
        char fichier[2048];
        if (!cheminFichier(fichier, sizeof fichier, base ? base : ".", "data")) goto fin;
        size_t n = strlen(fichier);
        SDL_snprintf(fichier + n, sizeof fichier - n, "/%s.bmp", noms[i]);
        sprites[i] = SDL_LoadBMP(fichier);
        if (!sprites[i]) {fprintf(stderr, "Image introuvable : %s\n", fichier); goto fin;}
    }
    if (!dossier && !smoke) preferences = SDL_GetPrefPath("SangFuu", "TowerDefense");
    if (!dossier) dossier = preferences ? preferences : ".";
    jeu = AlloueTab2D(LARGEURJEU, HAUTEURJEU);
    srand(seed);
    nouvellePartie(&roi, &horde, &chemin, jeu);
    int actif = 1, pause = 0, frames = 0;
    Uint32 dernierTour = SDL_GetTicks();
    const char *etat = "P : pause / N : rejouer / Echap : quitter";
    titre(fenetre, roi, pause, etat);
    while (actif) {
        SDL_Event evenement;
        while (SDL_PollEvent(&evenement)) {
            if (evenement.type == SDL_QUIT) actif = 0;
            if (evenement.type != SDL_KEYDOWN || evenement.key.repeat) continue;
            SDL_Keycode touche = evenement.key.keysym.sym;
            if (touche == SDLK_ESCAPE) actif = 0;
            else if (touche == SDLK_p || touche == SDLK_SPACE) pause = !pause;
            else if (touche == SDLK_n) {nouvellePartie(&roi, &horde, &chemin, jeu); pause = 0;}
            else if (touche == SDLK_s || touche == SDLK_c || touche == SDLK_d || touche == SDLK_v) {
                int binaire = touche == SDLK_s || touche == SDLK_c;
                int sauver = touche == SDLK_s || touche == SDLK_d;
                char fichier[2048];
                int ok = cheminFichier(fichier, sizeof fichier, dossier,
                                      binaire ? "partiebin.tdb" : "sauvegarde.tds");
                if (ok) ok = sauver ? sauvegarderPartie(fichier, binaire, chemin, roi, horde)
                                    : chargerPartie(fichier, binaire, chemin, &roi, &horde);
                reconstruire(jeu, roi, horde);
                etat = ok ? (sauver ? "Sauvegarde reussie" : "Partie restauree")
                          : "Operation impossible : fichier absent, invalide ou non accessible";
            }
            titre(fenetre, roi, pause, etat);
        }
        if (!actif) break;
        /* Dessiner d'abord les unites, puis les tirs, sans bloquer les evenements. */
        prepareAllSpriteDuJeu(jeu, chemin, LARGEURJEU, HAUTEURJEU, sprites, ecran);
        Uint32 maintenant = SDL_GetTicks();
        if (!pause && (smoke || maintenant - dernierTour >= 700)) {
            avancerPartie(&roi, &horde, chemin, jeu, ecran);
            dernierTour = maintenant;
            titre(fenetre, roi, pause, etat);
        }
        if (SDL_UpdateWindowSurface(fenetre) != 0) goto fin;
        if (smoke && ++frames == smoke) {
            SDL_Event quitter; SDL_zero(quitter); quitter.type = SDL_QUIT;
            if (SDL_PushEvent(&quitter) < 0) goto fin;
        }
        if (!smoke) SDL_Delay(16);
    }
    if (snapshot && SDL_SaveBMP(ecran, snapshot) != 0) goto fin;
    if (smoke) printf("Affichage SDL et fermeture : %d images verifiees.\n", frames);
    resultat = EXIT_SUCCESS;
fin:
    if (resultat != EXIT_SUCCESS) fprintf(stderr, "Erreur : %s\n", SDL_GetError());
    viderListe(&roi); viderListe(&horde); freeChemin(chemin); freePlateau(jeu, LARGEURJEU);
    for (int i = 0; i < 11; i++) SDL_FreeSurface(sprites[i]);
    SDL_free(base); SDL_free(preferences);
    /* La surface de fenetre appartient a SDL et ne doit pas etre liberee separement. */
    if (fenetre) SDL_DestroyWindow(fenetre);
    SDL_Quit();
    return resultat;
}
