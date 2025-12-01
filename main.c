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

// Fonction pour afficher le menu et retourner le mode choisi
int afficher_menu() {
    clear();

    // Afficher le titre
    afficher_titre("titre.txt");

    // Position pour le menu (sous le titre)
    int menu_y = 15;
    int menu_x = 30;

    // Affichage du menu
    attron(A_BOLD);
    mvprintw(menu_y, menu_x, "╔════════════════════════════════════════╗");
    mvprintw(menu_y + 1, menu_x, "║        SÉLECTIONNEZ UN MODE            ║");
    mvprintw(menu_y + 2, menu_x, "╠════════════════════════════════════════╣");
    mvprintw(menu_y + 3, menu_x, "║                                        ║");
    mvprintw(menu_y + 4, menu_x, "║  [1] Mode Normal                       ║");
    mvprintw(menu_y + 5, menu_x, "║      (Gestion fluide, 10 voitures max) ║");
    mvprintw(menu_y + 6, menu_x, "║                                        ║");
    mvprintw(menu_y + 7, menu_x, "║  [2] Mode Débordé                      ║");
    mvprintw(menu_y + 8, menu_x, "║      (Parking saturé, file d'attente)  ║");
    mvprintw(menu_y + 9, menu_x, "║                                        ║");
    mvprintw(menu_y + 10, menu_x, "║  [Q] Quitter                           ║");
    mvprintw(menu_y + 11, menu_x, "║                                        ║");
    mvprintw(menu_y + 12, menu_x, "╚════════════════════════════════════════╝");
    attroff(A_BOLD);

    refresh();

    // Attendre le choix de l'utilisateur
    int ch;
    while (1) {
        ch = getch();
        if (ch == '1') return 1;  // Mode normal
        if (ch == '2') return 2;  // Mode débordé
        if (ch == 'q' || ch == 'Q') return 0;  // Quitter
    }
}

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

    // Afficher le menu et obtenir le mode
    int mode = afficher_menu();
    if (mode == 0) {
        endwin();
        return 0;
    }

    wchar_t plan[max_ligne][max_colonne];
    VEHICULE* voitures = NULL;  // Liste chainée des voitures

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

    // Paramètres selon le mode
    int max_voitures, min_spawn_ticks, max_spawn_ticks;
    if (mode == 1) {
        // Mode Normal
        max_voitures = 10;
        min_spawn_ticks = 25;  // 5 secondes
        max_spawn_ticks = 51;  // 10 secondes
    } else {
        // Mode Débordé
        max_voitures = 30;     // Beaucoup plus de voitures
        min_spawn_ticks = 5;   // 1 seconde
        max_spawn_ticks = 16;  // 3 secondes (spawn très rapide)
    }

    // Tableau pour stocker les destinations de chaque voiture
    PLACE* targets[30] = {NULL};  // Augmenté pour mode débordé
    int nb_voitures_actives = 0;
    int nb_voitures_creees = 0;
    int nb_voitures_sorties = 0;
    // Spawn aléatoire
    int intervalle_spawn = min_spawn_ticks + rand() % (max_spawn_ticks - min_spawn_ticks);
    int tick_depuis_spawn = 0;

    // Boucle principale de simulation
    for (int t = 0; t < 2000; t++) {
        // Spawn d'une nouvelle voiture périodiquement
        tick_depuis_spawn++;
        if (tick_depuis_spawn >= intervalle_spawn && nb_voitures_actives < max_voitures) {
            PLACE* target = trouver_place_libre();
            if (target) {
                // IMPORTANT : Réserver la place immédiatement
                target->libre = 0;

                VEHICULE* nouvelle = creer_voiture('v', ENTREE_X, ENTREE_Y);
                ajouter_voiture(&voitures, nouvelle);

                // L'index de la nouvelle voiture est simplement nb_voitures_actives
                // (avant de l'incrémenter)
                targets[nb_voitures_actives] = target;

                nb_voitures_actives++;
                nb_voitures_creees++;
                tick_depuis_spawn = 0;
                // Nouveau intervalle aléatoire pour la prochaine voiture
                intervalle_spawn = min_spawn_ticks + rand() % (max_spawn_ticks - min_spawn_ticks);
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
                if (deplacer_vers_sortie(v, plan, voitures)) {
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
                if (target) deplacer_voiture_vers(v, target, plan, voitures);
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
                for (int i = idx; i < 29; i++) {
                    targets[i] = targets[i + 1];
                }
                targets[29] = NULL;

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
        if (mode == 1) {
            mvprintw(ligne_stats++, col_stats, "║  MODE: NORMAL    ║");
        } else {
            mvprintw(ligne_stats++, col_stats, "║  MODE: DÉBORDÉ   ║");
        }
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