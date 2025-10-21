#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "affichage.h"
#include "gestion.h"

void clear_screen() {
    printf("\033[2J\033[1;1H");
}

int main() {
    srand(time(NULL)); // Initialise le générateur aléatoire
    
    char plan[max_ligne][max_colonne];
    VEHICULE* voitures = NULL;
    
    // Affichage du titre et chargement du plan
    afficher_titre("titre.txt");
    charger_plan("parking.txt", plan);
    
    // Détecte les places de parking dans le plan
    trouver_places(plan);
    
    // Création d'une voiture à l'entrée
    VEHICULE* v1 = creer_voiture('v', ENTREE_X, ENTREE_Y);
    v1->ticks_gare = -1;
    v1->en_sortie = 0;
    ajouter_voiture(&voitures, v1);
    
    // Choisir la première place libre
    PLACE* target = trouver_place_libre();
    
    if (!target) {
        printf("Erreur: Aucune place de parking trouvée!\n");
        return 1;
    }
    
    // Place de sortie statique
    PLACE sortie = {SORTIE_X, SORTIE_Y, 1};
    
    // Boucle principale de simulation
    for (int t = 0; t < 500; t++) {
        clear_screen();
        charger_plan("parking.txt", plan);
        
        // ==========================
        // Gestion du déplacement
        // ==========================
        if (v1->etat == 1) { // Voiture en mouvement
            if (v1->en_sortie == 0) {
                // Déplacement vers la place libre
                deplacer_voiture_vers(v1, target);
                
                // Vérifie si arrivée à la place
                if (v1->posx == target->x && v1->posy == target->y) {
                    v1->etat = 0; // garée
                    v1->ticks_gare = 5 + rand() % 11; // 5 à 15 ticks
                    target->libre = 0; // place occupée
                }
            } else {
                // Déplacement vers la sortie
                deplacer_voiture_vers(v1, &sortie);
                
                // Arrivée à la sortie → init ticks pour rester 1 tick
                if (v1->posx == SORTIE_X && v1->posy == SORTIE_Y && v1->ticks_gare == -1) {
                    v1->ticks_gare = 1;
                }
                
                // Décrément du tick sur la sortie
                if (v1->posx == SORTIE_X && v1->posy == SORTIE_Y && v1->ticks_gare > 0) {
                    v1->ticks_gare--;
                }
                
                // Si tick terminé → disparaît
                if (v1->posx == SORTIE_X && v1->posy == SORTIE_Y && v1->ticks_gare == 0) {
                    v1->etat = 2; // disparait du plan
                }
            }
        } else if (v1->etat == 0 && v1->ticks_gare > 0) {
            // Voiture garée → décompte du tick
            v1->ticks_gare--;
        } else if (v1->etat == 0 && v1->ticks_gare == 0) {
            // Fin stationnement → départ vers sortie
            v1->etat = 1;
            v1->en_sortie = 1;
            target->libre = 1; // libère la place
        }
        
        // Dessine toutes les voitures et affiche le plan
        dessiner_voitures(voitures, plan);
        afficher_plan(plan, 50);
        
        // Pause entre frames
        usleep(150000);
    }
    
    // Libération mémoire
    while (voitures) {
        VEHICULE* tmp = voitures;
        voitures = voitures->NXT;
        free(tmp);
    }
    
    return 0;
}