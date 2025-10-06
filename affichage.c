#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "affichage.h"

void charger_plan(const char *filename,char plan[max_ligne][max_colonne]) {
    FILE *f=fopen(filename, "r");
    if(!f) { perror("Erreur ouverture fichier"); return; }

    int i=0;
    while (i < max_ligne && fgets(plan[i], max_colonne, f)) {
        plan[i][strcspn(plan[i], "\n")] = '\0';
        i++;
    }
    fclose(f);
}

void afficher_titre(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) { perror("Erreur ouverture titre"); return; }

    char ligne[300];
    while (fgets(ligne, sizeof(ligne), f)) {
        printf("%s", ligne);
        usleep(40000);
    }
    fclose(f);
    sleep(1);
}

void afficher_plan(char plan[max_ligne][max_colonne], int lignes) {
    for(int i=0;i<lignes;i++) printf("%s\n",plan[i]);
}

void dessiner_voiture(VEHICULE* v, char plan[max_ligne][max_colonne]) {
    if (!v) return;
    if (v->posx >= 0 && v->posx < max_ligne && v->posy >= 0 && v->posy < max_colonne)
        plan[v->posx][v->posy] = v->Carrosserie[0][0];
}

void dessiner_voitures(VEHICULE* liste, char plan[max_ligne][max_colonne]) {
    VEHICULE* tmp = liste;
    while (tmp) {
        if (tmp->etat != 2) dessiner_voiture(tmp, plan);
        tmp = tmp->NXT;
    }
}


