#define _XOPEN_SOURCE_EXTENDED 1 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <wchar.h>
#include <locale.h>
#include <ncursesw/curses.h>
#include "affichage.h"

// =======================================
// Chargement du plan depuis un fichier
// =======================================
void charger_plan(const char *filename, char plan[max_ligne][max_colonne]) {
    FILE *f = fopen(filename, "r");
    if (!f) { perror("Erreur ouverture fichier"); return; }

    int i = 0;
    while (i < max_ligne && fgets(plan[i], max_colonne, f)) {
        plan[i][strcspn(plan[i], "\n")] = '\0';
        i++;
    }
    fclose(f);
}

// =======================================
// Affichage du titre ASCII
// =======================================
void afficher_titre(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) { perror("Erreur ouverture titre"); return; }

    char ligne[300];
    while (fgets(ligne, sizeof(ligne), f)) {
        printw("%s", ligne);
        refresh();
        sleep(1); // 40ms comme effet de type "animation"
    }
    fclose(f);
    sleep(1);
    clear();
}

// =======================================
// Affichage du plan avec couleurs ncurses
// =======================================
void afficher_plan(char plan[max_ligne][max_colonne], int lignes) {
    clear();
    for (int i = 0; i < lignes; i++) {
        const char *p = plan[i];
        int j = 0;

        while (*p) {
            wchar_t wc;
            int len = mbtowc(&wc, p, MB_CUR_MAX); // UTF-8 -> wchar_t
            if (len <= 0) { // erreur ou fin
                break;
            }

            // Couleur selon type de caractère
            if (wc == L'■') attron(COLOR_PAIR(2)); // Rouge = occupé
            else if (wc == L'║' || wc == L'═' || wc == L'╔' || wc == L'╗' ||
                     wc == L'╚' || wc == L'╝' || wc == L'╩' || wc == L'╦' || wc == L'╬')
                attron(COLOR_PAIR(1)); // Vert = structure

            move(i, j);
            
            // Conversion wchar_t -> cchar_t pour add_wch
            cchar_t cch;
            wchar_t wstr[2] = {wc, L'\0'};
            setcchar(&cch, wstr, A_NORMAL, 0, NULL);
            add_wch(&cch);

            attroff(COLOR_PAIR(1));
            attroff(COLOR_PAIR(2));

            p += len;
            j++;
        }
    }
    refresh();
}

// =======================================
// Dessin d'une voiture
// =======================================
void dessiner_voiture(VEHICULE* v, char plan[max_ligne][max_colonne]) {
    if (!v) return;
    if (v->posx >= 0 && v->posx < max_ligne &&
        v->posy >= 0 && v->posy < (int)strlen(plan[v->posx])) {
        plan[v->posx][v->posy] = 'O';
    }
}

// =======================================
// Dessin de toutes les voitures
// =======================================
void dessiner_voitures(VEHICULE* liste, char plan[max_ligne][max_colonne]) {
    VEHICULE* tmp = liste;
    while (tmp) {
        dessiner_voiture(tmp, plan);
        tmp = tmp->NXT;
    }
}
