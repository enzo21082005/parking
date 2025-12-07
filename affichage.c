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
        perror("Erreur ouverture fichier"); 
        return; 
    }

    // Important : définir le mode wide pour lire des wchar_t
    fwide(f, 1);

    int x = 0, y = 0;
    wint_t c;
    
    // Initialiser toutes les lignes
    for (int i = 0; i < max_ligne; i++) {
        plan[i][0] = L'\0';
    }
    
    // Lire caractère par caractère (wide char)
    while ((c = fgetwc(f)) != WEOF && y < max_ligne) {
        if (c == L'\n') {
            // Fin de ligne : terminer la ligne actuelle
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
// Affichage du plan (VERSION CORRIGÉE)
// =======================================
void afficher_plan(wchar_t plan[max_ligne][max_colonne], int lignes) {
    clear(); // Important : effacer l'écran avant d'afficher
    
    for (int i = 0; i < lignes; i++) {
        // Vérifier que la ligne n'est pas vide
        if (plan[i][0] == L'\0') {
            continue; // Sauter les lignes vides
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
    while (fgets(ligne, sizeof(ligne), f)) {
        printw("%s", ligne);
    }
    fclose(f);
    refresh();
    // Affichage instantané du titre
}


// =======================================
// Dessin d'une voiture (VERSION AVEC ORIENTATION)
// =======================================
void dessiner_voiture(VEHICULE* v) {
    if (!v) return;

    // On active la couleur ROUGE (paire 2)
    attron(COLOR_PAIR(2));

    // Choix du symbole selon la direction
    wchar_t symbole;
    switch(v->direction) {
        case 'N':  // Nord (haut)
            symbole = L'▲';
            break;
        case 'S':  // Sud (bas)
            symbole = L'▼';
            break;
        case 'E':  // Est (droite)
            symbole = L'►';
            break;
        case 'W':  // Ouest (gauche)
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
// Dessin de toutes les voitures (MISE À JOUR)
// =======================================
void dessiner_voitures(VEHICULE* liste) {
    VEHICULE* tmp = liste;
    while (tmp) {
        // La fonction ne prend plus le 'plan' en paramètre
        dessiner_voiture(tmp);
        tmp = tmp->NXT;
    }
}
