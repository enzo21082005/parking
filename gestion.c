#include <stdio.h>
#include <stdlib.h>
#include "gestion.h"

#define NB_PLACES 16
PLACE places[NB_PLACES];
int nb_places = 0;

// =============================
// Création et gestion des voitures
// =============================
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

    v->Carrosserie[0][0] = 'O';
    v->Carrosserie[0][1] = '\0';

    return v;
}

void ajouter_voiture(VEHICULE** liste, VEHICULE* v) {
    if (!*liste) {
        *liste = v;
    } else {
        VEHICULE* tmp = *liste;
        while (tmp->NXT) tmp = tmp->NXT;
        tmp->NXT = v;
    }
}

// =============================
// Gestion des places
// =============================
void trouver_places(char plan[max_ligne][max_colonne]) {
    nb_places = 0;
    for (int i = 0; i < max_ligne; i++) {
        for (int j = 0; j < max_colonne; j++) {
            if (plan[i][j] == '[' && plan[i][j+2] == ']') {
                places[nb_places].x = i;
                places[nb_places].y = j;
                places[nb_places].libre = 1;
                nb_places++;
                if (nb_places >= NB_PLACES) break;
            }
        }
    }
}

PLACE* trouver_place_libre() {
    for (int i = 0; i < nb_places; i++) {
        if (places[i].libre) return &places[i];
    }
    return NULL;
}

// =============================
// Déplacement voiture
// =============================
void deplacer_voiture_vers(VEHICULE* v, PLACE* target) 
{
    if (!v || !target) return;

    if (v->posx < target->x) v->posx++;
    else if (v->posx > target->x) v->posx--;

    if (v->posy < target->y) v->posy++;
    else if (v->posy > target->y) v->posy--;

    // Arrivée sur la place
    if (v->posx == target->x && v->posy == target->y && v->en_sortie == 0) {
        if (v->ticks_gare == -1) {
            v->ticks_gare = 5 + rand() % 11; // 5 à 15 ticks
            target->libre = 0;
            v->etat = 0;
            v->posy = target->y + 1;
        }
    }
}

