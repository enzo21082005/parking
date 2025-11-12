#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

    afficher_titre("titre.txt");
    charger_plan("parking.txt", plan);

    trouver_places(plan);

    VEHICULE* v1 = creer_voiture('v', ENTREE_X, ENTREE_Y);
    ajouter_voiture(&voitures, v1);
    PLACE* target = trouver_place_libre();

    for (int t = 0; t < 300; t++) {
        charger_plan("parking.txt", plan);

        char plan_temp[max_ligne][max_colonne];
        memcpy(plan_temp, plan, sizeof(plan_temp));

        if (target) deplacer_voiture_vers(v1, target);

        dessiner_voitures(voitures, plan_temp);
        afficher_plan(plan_temp, 50);

        sleep(1); // pause animation
    }

    endwin();
    return 0;
}
