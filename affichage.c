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
void charger_plan(const char *filename, wchar_t plan[max_ligne][max_colonne]) {
    FILE *f = fopen(filename, "r");
    if (!f) { 
        perror("Erreur ouverture fichier"); // erreur si on ne trouve pas le fichier
        return; 
    }
    fwide(f, 1);

    int x = 0, y = 0;
    wint_t c;
    
    for (int i = 0; i < max_ligne; i++) {
        plan[i][0] = L'\0';
    }
    
    // Lire caractère par caractère (wide char)
    while ((c = fgetwc(f)) != WEOF && y < max_ligne) {
        if (c == L'\n') {
            plan[y][x] = L'\0';
            y++;  // Passer à la ligne suivante
            x = 0; // Remettre la colonne à 0
        } 
        else if (c == L'\r') {
            // Ignorer les retours chariot (Windows)
            continue;
        }
        else if (x < max_colonne - 1) {
            plan[y][x++] = (wchar_t)c;
        }
    }
    
    // Terminer la dernière ligne si elle existe
    if (y < max_ligne) {
        plan[y][x] = L'\0';
    }
    
    fclose(f);
}

// =======================================
// Affichage du plan
// =======================================
void afficher_plan(wchar_t plan[max_ligne][max_colonne], int lignes) {
    clear(); //effacer l'écran avant d'afficher
    
    for (int i = 0; i < lignes; i++) {
        if (plan[i][0] == L'\0') {
            continue;
        }
        
        for (int j = 0; plan[i][j] != L'\0' && j < max_colonne; j++) {
            wchar_t wc = plan[i][j];
            
            // Détermine la couleur
            int pair = 0;
            if (wc == L'■') {
                pair = 2; // Rouge pour les places
            } else if (wc == L'B') {
                pair = 3; // Jaune pour la borne (on va créer cette couleur)
            } else if (wc == L'║' || wc == L'═' || wc == L'╔' || wc == L'╗' ||
                       wc == L'╚' || wc == L'╝' || wc == L'╩' || wc == L'╦' ||
                       wc == L'╬' || wc == L'►' || wc == L'◄' || wc == L'─' ||
                       wc == L'│' || wc == L'█') {
                pair = 1; // Vert pour la structure
            }

            // Crée et affiche le caractère
            cchar_t cch;
            wchar_t wstr[2] = {wc, L'\0'};
            setcchar(&cch, wstr, A_NORMAL, pair, NULL);
            mvadd_wch(i, j, &cch);
        }
    }
    refresh();
}

void afficher_titre(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) { perror("Erreur ouverture titre"); return; }

    char ligne[300];
    while (fgets(ligne, sizeof(ligne), f)) { // Lecture du fichier
        printw("%s", ligne);
    }
    fclose(f);
    refresh();
    // Affichage du titre
}


// =======================================
// Dessin d'une voiture
// =======================================
void dessiner_voiture(VEHICULE* v) {
    if (!v) return;

    // On active la couleur ROUGE
    attron(COLOR_PAIR(2));

    // Choix du symbole selon la direction
    wchar_t symbole;
    switch(v->direction) {
        case 'N':  // Nord 
            symbole = L'▲';
            break;
        case 'S':  // Sud 
            symbole = L'▼';
            break;
        case 'E':  // Est 
            symbole = L'►';
            break;
        case 'W':  // Ouest 
            symbole = L'◄';
            break;
        default:
            symbole = L'●';  // Par défaut
            break;
    }

    // Créer et afficher le caractère wide
    cchar_t cch;
    wchar_t wstr[2] = {symbole, L'\0'};
    setcchar(&cch, wstr, A_BOLD, 2, NULL);
    mvadd_wch(v->posx, v->posy, &cch);

    // On désactive la couleur
    attroff(COLOR_PAIR(2));
}

// =======================================
// Dessin de toutes les voitures
// =======================================
void dessiner_voitures(VEHICULE* liste) {
    VEHICULE* tmp = liste;
    while (tmp) {
        dessiner_voiture(tmp);
        tmp = tmp->NXT;
    }
}
