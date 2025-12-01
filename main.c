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
    // === Initialisation ===
    srand(time(NULL));  // Initialiser le générateur aléatoire
    setlocale(LC_ALL, "");
    initscr();
    start_color();
    noecho();
    curs_set(FALSE);
    keypad(stdscr, TRUE);
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);

    wchar_t plan[max_ligne][max_colonne];
    VEHICULE* voitures = NULL;  // Liste chainée des voitures

    /*afficher_titre("titre.txt");*/
    charger_plan("parking.txt", plan);
    trouver_places(plan);

    int hauteur_plan = 0;
    int largeur_plan = 0;
    while (hauteur_plan < max_ligne && plan[hauteur_plan][0] != L'\0') {
        int w = 0;
        while (plan[hauteur_plan][w] != L'\0') w++;
        if (w > largeur_plan) largeur_plan = w;
        hauteur_plan++;
    }

    // Tableau pour stocker les destinations de chaque voiture
    PLACE* targets[10] = {NULL};
    int nb_voitures_actives = 0;
    int nb_voitures_creees = 0;
    int nb_voitures_sorties = 0;
    // Spawn aléatoire entre 5-10 secondes = 25-50 frames (à 200ms/frame)
    int intervalle_spawn = 25 + rand() % 26;
    int tick_depuis_spawn = 0;

    // Boucle principale de simulation
    for (int t = 0; t < 2000; t++) {
        // Spawn d'une nouvelle voiture périodiquement
        tick_depuis_spawn++;
        if (tick_depuis_spawn >= intervalle_spawn && nb_voitures_actives < 10) {
            PLACE* target = trouver_place_libre();
            if (target) {
                VEHICULE* nouvelle = creer_voiture('v', ENTREE_X, ENTREE_Y);
                ajouter_voiture(&voitures, nouvelle);

                // L'index de la nouvelle voiture est simplement nb_voitures_actives
                // (avant de l'incrémenter)
                targets[nb_voitures_actives] = target;

                nb_voitures_actives++;
                nb_voitures_creees++;
                tick_depuis_spawn = 0;
                // Nouveau intervalle aléatoire pour la prochaine voiture
                intervalle_spawn = 25 + rand() % 26;
            }
        }

        // Gérer toutes les voitures actives
        VEHICULE* v = voitures;
        VEHICULE* prev = NULL;
        int idx = 0;

        while (v) {
            VEHICULE* next = v->NXT;
            PLACE* target = targets[idx];
            int supprimer = 0;

            // Logique de déplacement
            if (v->en_sortie) {
                // En route vers la sortie
                if (deplacer_vers_sortie(v, plan)) {
                    supprimer = 1;
                }
            } else if (v->etat == 0 && v->ticks_gare > 0) {
                // Garée, décrémenter le temps
                v->ticks_gare--;

                // Compteur de temps : 1€ toutes les 50 ticks
                if (v->ticks_gare % 50 == 0) {
                    v->argent_du++;
                }

                if (v->ticks_gare == 0) {
                    // Temps écoulé, libérer et partir
                    if (target) target->libre = 1;
                    v->en_sortie = 1;
                    v->etat = 1;
                }
            } else if (v->etat == 1 && !v->en_sortie) {
                // En mouvement vers la place
                if (target) deplacer_voiture_vers(v, target, plan);
            }

            // Supprimer la voiture si elle est sortie
            if (supprimer) {
                // Collecter le paiement avant de supprimer la voiture
                argent_total += v->argent_du;

                if (prev) {
                    prev->NXT = next;
                } else {
                    voitures = next;
                }

                // Décaler les targets
                for (int i = idx; i < 9; i++) {
                    targets[i] = targets[i + 1];
                }
                targets[9] = NULL;

                free(v);
                nb_voitures_actives--;
                nb_voitures_sorties++;
            } else {
                prev = v;
                idx++;
            }

            v = next;
        }

        // Affichage du parking
        afficher_plan(plan, hauteur_plan);
        dessiner_voitures(voitures);

        // Compter les voitures et les places
        int count_voitures = 0;
        int count_garees = 0;
        VEHICULE* tmp = voitures;
        while (tmp) {
            count_voitures++;
            if (tmp->etat == 0) count_garees++;  // Voitures garées
            tmp = tmp->NXT;
        }

        int places_libres = 0;
        for (int i = 0; i < nb_places; i++) {
            if (places[i].libre) places_libres++;
        }

        // Affichage des statistiques À DROITE du parking (version compacte)
        int col_stats = largeur_plan + 1;  // 1 colonne d'espacement
        int ligne_stats = 0;

        mvprintw(ligne_stats++, col_stats, "╔══════════════════╗");
        mvprintw(ligne_stats++, col_stats, "║   STATISTIQUES   ║");
        mvprintw(ligne_stats++, col_stats, "╠══════════════════╣");
        mvprintw(ligne_stats++, col_stats, "║ Act.  : %-3d      ║", count_voitures);
        mvprintw(ligne_stats++, col_stats, "║ Garé  : %-3d      ║", count_garees);
        mvprintw(ligne_stats++, col_stats, "║ Ent.  : %-3d      ║", nb_voitures_creees);
        mvprintw(ligne_stats++, col_stats, "║ Sort. : %-3d      ║", nb_voitures_sorties);
        mvprintw(ligne_stats++, col_stats, "║ Libre : %-2d/%-2d    ║", places_libres, nb_places);
        mvprintw(ligne_stats++, col_stats, "║ Spawn : %.1fs     ║",
                 (intervalle_spawn - tick_depuis_spawn) * 0.2);
        mvprintw(ligne_stats++, col_stats, "║ €     : %-4d     ║", argent_total);
        mvprintw(ligne_stats++, col_stats, "╚══════════════════╝");

        refresh();

        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 200000000;  // 200ms
        nanosleep(&ts, NULL);
    }

    // Libérer toutes les voitures restantes
    while (voitures) {
        VEHICULE* tmp = voitures;
        voitures = voitures->NXT;
        free(tmp);
    }

    endwin();
    return 0;
}
