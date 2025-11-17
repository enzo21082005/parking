#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <wchar.h>
#include <locale.h>
#include <ncursesw/curses.h>
#include "affichage.h"
#include "gestion.h"


int main() {
    // === Initialisation de ncurses ===
    setlocale(LC_ALL, "");
    initscr();
    start_color();
    noecho();
    curs_set(FALSE);
    keypad(stdscr, TRUE);
    init_pair(1, COLOR_GREEN, COLOR_BLACK); // structure
    init_pair(2, COLOR_RED, COLOR_BLACK);   // voitures

    char plan[max_ligne][max_colonne];
    VEHICULE* voitures = NULL;

    /*afficher_titre("titre.txt");*/
    charger_plan("parking.txt", plan);

    trouver_places(plan);

    int hauteur_plan = 0;
    while (hauteur_plan < max_ligne && plan[hauteur_plan][0] != '\0') {
        hauteur_plan++;
    }

    VEHICULE* v1 = creer_voiture('v', ENTREE_X, ENTREE_Y);
    ajouter_voiture(&voitures, v1);
    PLACE* target = trouver_place_libre();

    for (int t = 0; t < 300; t++) {
        if (target) deplacer_voiture_vers(v1, target, plan);

        
        // === 2. AFFICHAGE ===
        
        // 1. On efface TOUT l'écran
        
        
        // 2. On dessine le FOND (le parking)
        afficher_plan(plan, hauteur_plan);
        
        // 3. On dessine les VOITURES par-dessus le fond
        dessiner_voitures(voitures);
        
        // 4. On affiche le résultat final à l'écran
        refresh();

        struct timespec ts;
        // Pause de 0.1 secondes (100 millisecondes)
        ts.tv_sec = 0;           // Secondes
        ts.tv_nsec = 100000000;  // 100 millions de nanosecondes (0.1s)
        nanosleep(&ts, NULL);
    }

    endwin();
    return 0;
}