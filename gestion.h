#ifndef GESTION_H
#define GESTION_H
#include <wchar.h>

#define max_ligne 100
#define max_colonne 200

typedef struct voiture VEHICULE;
struct voiture {
    char direction;
    int posx, posy;
    char type;
    char Carrosserie[4][30];
    char etat;
    int ticks_gare;
    char en_sortie;
    int argent_du;  // Argent accumulé par cette voiture
    int ticks_a_borne;  // Temps passé à la borne de paiement
    struct voiture *NXT;
};

typedef struct {
    int x, y;
    int libre;
} PLACE;

#define ENTREE_X 8
#define ENTREE_Y 112
#define BORNE_X 5
#define BORNE_Y 6  // Position de la borne de paiement
#define SORTIE_X 6
#define SORTIE_Y 3  // Position de la sortie (le 'S')
#define TICKS_PAIEMENT 10  // 2 secondes à 200ms/tick

// Variables globales pour les places
extern PLACE places[];
extern int nb_places;

// Variable globale pour l'argent total gagné
extern int argent_total;

VEHICULE* creer_voiture(char type, int x, int y);
void ajouter_voiture(VEHICULE** liste, VEHICULE* v);
void trouver_places(wchar_t plan[max_ligne][max_colonne]);
PLACE* trouver_place_libre();
int est_un_espace(int x, int y, wchar_t plan[max_ligne][max_colonne]);
int position_occupee(int x, int y, VEHICULE* liste, VEHICULE* ignore);
void deplacer_voiture_vers(VEHICULE* v, PLACE* target, wchar_t plan[max_ligne][max_colonne], VEHICULE* liste);
int deplacer_vers_sortie(VEHICULE* v, wchar_t plan[max_ligne][max_colonne], VEHICULE* liste);

#endif

