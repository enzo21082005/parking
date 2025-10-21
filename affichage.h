#ifndef AFFICHAGE_H
#define AFFICHAGE_H
#include "gestion.h"

#define max_ligne 100
#define max_colonne 200


void charger_plan(const char *filename,char plan[max_ligne][max_colonne]);
void afficher_plan(char plan[max_ligne][max_colonne], VEHICULE* liste,int ligne);
void afficher_titre(const char *filename);
void dessiner_voiture(VEHICULE* v, char plan[max_ligne][max_colonne]);
void dessiner_voitures(VEHICULE* liste, char plan[max_ligne][max_colonne]);

#endif


