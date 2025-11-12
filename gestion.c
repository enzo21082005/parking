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

void deplacer_voiture_vers(VEHICULE* v, PLACE* target) {
    if (!v || !target) return;

    if (v->posx < target->x) v->posx++;
    else if (v->posx > target->x) v->posx--;

    if (v->posy < target->y) v->posy++;
    else if (v->posy > target->y) v->posy--;

    if (v->posx == target->x && v->posy == target->y) {
        if (v->ticks_gare == -1) {
            v->ticks_gare = 5 + rand() % 10;
            target->libre = 0;
            v->etat = 0;
        }
    }
}
