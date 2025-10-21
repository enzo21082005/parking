#ifndef GESTION_H
#define GESTION_H

#define max_ligne 100
#define max_colonne 200

typedef struct voiture VEHICULE;
struct voiture {
    char direction;
    int posx, posy;
    char type;
    char Carrosserie[4][30];
    char etat;       // 1 = en mouvement, 0 = garée
    int ticks_gare;  // nombre de ticks avant départ
    char en_sortie;  // 0 = vers place, 1 = vers sortie
    struct voiture *NXT;
};

// Structure pour représenter une place de parking
typedef struct {
    int x, y;
    int libre; // 1 = libre, 0 = occupée
    int marqueur_x;  // Position X du marqueur
    int marqueur_y;  // Position Y du marqueur
} PLACE;

// Coordonnées entrée/sortie
#define ENTREE_X 8
#define ENTREE_Y 107
#define SORTIE_X 7
#define SORTIE_Y 1

// Fonctions voitures
VEHICULE* creer_voiture(char type, int x, int y);
void ajouter_voiture(VEHICULE** liste, VEHICULE* v);

// Fonctions places
void trouver_places(char plan[max_ligne][max_colonne]);
PLACE* trouver_place_libre();

// Déplacement
void deplacer_voiture_vers(VEHICULE* v, PLACE* target);

#endif

