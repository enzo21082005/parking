#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <wchar.h>
#include <locale.h>
#include <ncursesw/curses.h>
#include <signal.h>
#include <sys/wait.h>
#include "affichage.h"
#include "gestion.h"

// Variables globales pour la musique
pid_t music_pid = -1;
int music_enabled = 1;  // 1 = activé, 0 = désactivé

// =======================================
// Démarre la musique de fond en boucle
// =======================================
void start_music() {
    if (music_pid != -1) return;  // Déjà en cours

    music_pid = fork();
    if (music_pid == 0) {
        // Processus enfant - jouer la musique en boucle
        // Rediriger stderr et stdout vers /dev/null pour éviter les messages
        freopen("/dev/null", "w", stdout);
        freopen("/dev/null", "w", stderr);

        // Boucle infinie pour rejouer la musique
        while (1) {
            // Essayer mpg123 d'abord (pour MP3)
            execlp("mpg123", "mpg123", "-q", "elevator_music.mp3", NULL);
            // Si mpg123 échoue, essayer aplay (pour WAV)
            execlp("aplay", "aplay", "-q", "elevator_music.wav", NULL);
            // Si les deux échouent, sortir
            exit(0);
        }
    }
}

// =======================================
// Arrête la musique de fond
// =======================================
void stop_music() {
    if (music_pid > 0) {
        kill(music_pid, SIGTERM);
        waitpid(music_pid, NULL, 0);
        music_pid = -1;
    }
}

// =======================================
// Active ou désactive la musique
// =======================================
void toggle_music() {
    music_enabled = !music_enabled;
    if (music_enabled) {
        start_music();
    } else {
        stop_music();
    }
}

// =======================================
// Affiche le menu principal et retourne le mode choisi
// =======================================
int afficher_menu() {
    clear();

    // Dessiner un fond de parking décoratif
    attron(COLOR_PAIR(1));

    // Bordure supérieure
    mvprintw(0, 0, "╔═══════════════════════════════════════════════════════════════════════════════════════════════════╗");

    // Lignes de fond avec motifs de parking
    for (int i = 1; i < 30; i++) {
        mvprintw(i, 0, "║");
        // Motif de places de parking sur les côtés
        if (i >= 14 && i <= 26 && i % 2 == 0) {
            mvprintw(i, 3, "■");
            mvprintw(i, 93, "■");
        }
        mvprintw(i,100, "║");
    }

    // Lignes de route au milieu
    for (int i = 14; i < 27; i++) {
        mvprintw(i, 10, "─────");
        mvprintw(i, 85, "─────");
    }

    // Bordure inférieure
    mvprintw(30, 0, "╚═══════════════════════════════════════════════════════════════════════════════════════════════════╝");
    attroff(COLOR_PAIR(1));

    // Afficher le titre CENTRÉ en haut
    FILE *f = fopen("titre.txt", "r");
    if (f) {
        char ligne[300];
        int y = 2;
        while (fgets(ligne, sizeof(ligne), f) && y < 11) {
            // Calculer la position pour centrer (environ colonne 20 pour un titre de 70 caractères)
            int x = 15;
            attron(A_BOLD | COLOR_PAIR(2));
            mvprintw(y, x, "%s", ligne);
            attroff(A_BOLD | COLOR_PAIR(2));
            y++;
        }
        fclose(f);
    }

    // Position pour le menu (centré)
    int menu_y = 15;
    int menu_x = 30;

    // Affichage du menu
    while (1) {
        attron(A_BOLD | COLOR_PAIR(1));
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
        mvprintw(menu_y + 10, menu_x, "║  [M] Musique: %-3s                      ║",
                 music_enabled ? "ON" : "OFF");
        mvprintw(menu_y + 11, menu_x, "║  [Q] Quitter                           ║");
        mvprintw(menu_y + 12, menu_x, "║                                        ║");
        mvprintw(menu_y + 13, menu_x, "╚════════════════════════════════════════╝");
        attroff(A_BOLD | COLOR_PAIR(1));

        refresh();

        // Attendre le choix de l'utilisateur
        int ch = getch();
        if (ch == '1') return 1;  // Mode normal
        if (ch == '2') return 2;  // Mode débordé
        if (ch == 'm' || ch == 'M') {
            toggle_music();
            // Continuer la boucle pour réafficher le menu avec le nouveau statut
        } else if (ch == 'q' || ch == 'Q') {
            return 0;  // Quitter
        }
    }
}

// =======================================
// Fonction principale - simulation du parking
// =======================================
int main() {
    // Initialisation de ncurses et des couleurs
    srand(time(NULL));
    setlocale(LC_ALL, "");
    initscr();
    start_color();
    noecho();
    curs_set(FALSE);
    keypad(stdscr, TRUE);
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);  // Jaune pour la borne

    // Démarrer la musique au lancement
    start_music();

    // Boucle principale pour permettre de retourner au menu
    while (1) {
        // Afficher le menu et obtenir le mode
        int mode = afficher_menu();
        if (mode == 0) {
            stop_music();  // Arrêter la musique avant de quitter
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

    // Activer le mode non-bloquant pour getch()
    nodelay(stdscr, TRUE);

    // Variable pour contrôler le retour au menu
    int retour_menu = 0;

    // Boucle principale de simulation
    for (int t = 0; t < 2000 && !retour_menu; t++) {
        // Vérifier les touches appuyées
        int ch = getch();
        if (ch == 'm' || ch == 'M') {
            toggle_music();
        } else if (ch == 'q' || ch == 'Q' || ch == 27) {  // Q ou Echap (27)
            retour_menu = 1;
            break;  // Sortir de la boucle pour retourner au menu
        }
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
        mvprintw(ligne_stats++, col_stats, "╠══════════════════╣");
        mvprintw(ligne_stats++, col_stats, "║ Music : %-3s [M]  ║",
                 music_enabled ? "ON" : "OFF");
        mvprintw(ligne_stats++, col_stats, "║ Menu  : [Q/ESC]  ║");
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

        // Réinitialiser l'argent total pour la prochaine session
        argent_total = 0;

        // Si on a quitté avec Q/Echap, retourner au menu
        // Sinon, la boucle while(1) continuera automatiquement
    }

    // Cette partie n'est jamais atteinte car la boucle while(1) est infinie
    // Le programme se termine uniquement via le menu avec mode == 0
    stop_music();
    endwin();
    return 0;
}