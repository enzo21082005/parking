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
    while (hauteur_plan < max_ligne && plan[hauteur_plan][0] != L'\0') {
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

                // Trouver l'index de cette voiture
                VEHICULE* tmp = voitures;
                int idx = 0;
                while (tmp->NXT) {
                    tmp = tmp->NXT;
                    idx++;
                }
                targets[idx] = target;

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

                // Paiement : 1€ toutes les 50 ticks
                if (v->ticks_gare % 50 == 0) {
                    v->argent_du++;
                    argent_total++;
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

        // Affichage des statistiques EN DESSOUS du parking
        int ligne_info = hauteur_plan + 1;
        mvprintw(ligne_info, 0, "═══════════════════════════════════════════════════════════════════════════");
        ligne_info++;
        mvprintw(ligne_info, 0, "  STATISTIQUES DU PARKING");
        ligne_info++;
        mvprintw(ligne_info, 0, "═══════════════════════════════════════════════════════════════════════════");
        ligne_info++;
        mvprintw(ligne_info, 0, "  Voitures actives      : %d", count_voitures);
        ligne_info++;
        mvprintw(ligne_info, 0, "  Voitures garées       : %d", count_garees);
        ligne_info++;
        mvprintw(ligne_info, 0, "  Total entrées         : %d", nb_voitures_creees);
        ligne_info++;
        mvprintw(ligne_info, 0, "  Total sorties         : %d", nb_voitures_sorties);
        ligne_info++;
        mvprintw(ligne_info, 0, "  Places libres         : %d / %d", places_libres, nb_places);
        ligne_info++;
        mvprintw(ligne_info, 0, "  Prochain spawn dans   : %.1f secondes",
                 (intervalle_spawn - tick_depuis_spawn) * 0.2);
        ligne_info++;
        mvprintw(ligne_info, 0, "  Argent gagné          : %d €", argent_total);
        ligne_info++;
        mvprintw(ligne_info, 0, "═══════════════════════════════════════════════════════════════════════════");

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