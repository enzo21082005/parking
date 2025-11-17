#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gestion.h"
#include <wchar.h>
#include <locale.h>
#include <ncursesw/curses.h>


#define NB_PLACES 48
PLACE places[NB_PLACES];
int nb_places = 0;

VEHICULE* creer_voiture(char type, int x, int y) {
    VEHICULE* v = malloc(sizeof(VEHICULE));
    if (!v) return NULL;

    v->direction = 'N';
    v->posx = x;
    v->posy = y;
    v->type = type;
    v->etat = 1;
    v->ticks_gare = -1;
    v->en_sortie = 0;
    v->NXT = NULL;

    return v;
}

void ajouter_voiture(VEHICULE** liste, VEHICULE* v) {
    if (!*liste) *liste = v;
    else {
        VEHICULE* tmp = *liste;
        while (tmp->NXT) tmp = tmp->NXT;
        tmp->NXT = v;
    }
}

void trouver_places(char plan[max_ligne][max_colonne]) {
    nb_places = 0;

    for (int i = 0; i < max_ligne && nb_places < NB_PLACES; i++) {
        const char *p = plan[i];
        int j = 0;

        while (*p && nb_places < NB_PLACES) {
            wchar_t wc;
            int len = mbtowc(&wc, p, MB_CUR_MAX);

            if (len <= 0)
                break;

            if (wc == L'■') {
                places[nb_places].x = i;
                places[nb_places].y = j;
                places[nb_places].libre = 1;
                nb_places++;
            }

            p += len;
            j++;
        }
    }
}


PLACE* trouver_place_libre() {
    for (int i = 0; i < nb_places; i++) {
        if (places[i].libre) return &places[i];
    }
    return NULL;
}

int est_un_espace(int x, int y, char plan[max_ligne][max_colonne]) {
    if (x < 0 || x >= max_ligne) return 0;
    
    int j_char = 0;   // Compteur de caractères
    int byte_idx = 0; // Compteur de bytes
    char *ligne = plan[x];

    // Avance dans la chaîne (byte par byte) jusqu'à trouver la colonne (j_char)
    while (ligne[byte_idx] != '\0' && j_char < y) {
        wchar_t wc;
        int len = mbtowc(&wc, &ligne[byte_idx], MB_CUR_MAX);
        if (len <= 0) return 0; // Erreur de lecture
        byte_idx += len;
        j_char++;
    }

    // Si on a trouvé la bonne colonne
    if (j_char == y && ligne[byte_idx] != '\0') {
        wchar_t wc;
        mbtowc(&wc, &ligne[byte_idx], MB_CUR_MAX);
        // Renvoie VRAI (1) si c'est un espace
        return (wc == L' ' || wc == L'►' || wc == L'◄'); // Autorise aussi les flèches
    }
    
    return 0; // Pas trouvé ou fin de ligne
}

void deplacer_voiture_vers(VEHICULE* v, PLACE* target, char plan[max_ligne][max_colonne]) {
    if (!v || !target) return;

    // -----
    // PHASE 1: On est arrivé à la place
    // -----
    if (v->posx == target->x && v->posy == target->y) {
        if (v->ticks_gare == -1) {
            v->ticks_gare = 5 + rand() % 10;
            target->libre = 0;
            v->etat = 0; // La voiture est garée
        }
        return; // On ne bouge plus
    }

    // -----
    // PHASE 2: On n'est pas arrivé, on calcule le prochain mouvement
    // -----
    
    // On essaie de se rapprocher sur l'axe Y (horizontal)
    if (v->posy != target->y) {
        if (v->posy < target->y) { // Veut aller à droite
            if (est_un_espace(v->posx, v->posy + 1, plan)) {
                v->posy++;
                return; // Mouvement effectué, on arrête pour ce tick
            }
        } else { // Veut aller à gauche
            if (est_un_espace(v->posx, v->posy - 1, plan)) {
                v->posy--;
                return; // Mouvement effectué
            }
        }
    }

    // Si on n'a pas pu bouger en Y (ou si on est aligné), on essaie en X (vertical)
    if (v->posx != target->x) {
        if (v->posx < target->x) { // Veut aller en bas
            if (est_un_espace(v->posx + 1, v->posy, plan)) {
                v->posx++;
                return; // Mouvement effectué
            }
        } else { // Veut aller en haut
            if (est_un_espace(v->posx - 1, v->posy, plan)) {
                v->posx--;
                return; // Mouvement effectué
            }
        }
    }

    // Si on est là, c'est qu'on est bloqué (par ex. aligné en X mais pas en Y,
    // et bloqué en Y). C'est un cas simple de "pathfinding" qui coince.
    // Pour un vrai projet, il faudrait un algo A*, mais pour l'instant
    // ça devrait te débloquer pour 90% des cas.
}
