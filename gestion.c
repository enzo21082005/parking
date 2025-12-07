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
int argent_total = 0;  // Argent total gagné par le parking

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
    v->argent_du = 0;  // Initialiser l'argent à 0
    v->ticks_a_borne = -1;  // -1 = pas encore à la borne
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

void trouver_places(wchar_t plan[max_ligne][max_colonne]) {
    nb_places = 0;

    for (int i = 0; i < max_ligne && nb_places < NB_PLACES; i++) {
        for (int j = 0; j < max_colonne && plan[i][j] != L'\0'; j++) {
            if (plan[i][j] == L'■') {
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
    // Compter les places libres
    int nb_libres = 0;
    for (int i = 0; i < nb_places; i++) {
        if (places[i].libre) nb_libres++;
    }

    if (nb_libres == 0) return NULL;

    // Choisir un index aléatoire parmi les places libres
    int choix = rand() % nb_libres;

    // Trouver la place correspondante
    int compte = 0;
    for (int i = 0; i < nb_places; i++) {
        if (places[i].libre) {
            if (compte == choix) return &places[i];
            compte++;
        }
    }

    return NULL;
}

int est_un_espace(int x, int y, wchar_t plan[max_ligne][max_colonne]) {
    if (x < 0 || x >= max_ligne) return 0;
    if (y < 0 || y >= max_colonne) return 0;

    wchar_t wc = plan[x][y];

    // DEBUG: Afficher le caractère pour comprendre ce qui bloque
    // fprintf(stderr, "Checking (%d,%d) = '%lc' (code: %d)\n", x, y, wc, (int)wc);

    // Espaces praticables : espaces vides, flèches d'entrée/sortie, 'S' pour sortie, et 'B' pour borne
    return (wc == L' ' || wc == L'►' || wc == L'◄' || wc == L'S' || wc == L'B');
}

// Version étendue qui autorise aussi les places de parking
int est_accessible(int x, int y, int goal_x, int goal_y, wchar_t plan[max_ligne][max_colonne]) {
    if (x < 0 || x >= max_ligne) return 0;
    if (y < 0 || y >= max_colonne) return 0;

    wchar_t wc = plan[x][y];

    // Si c'est la destination finale, autoriser les places
    if (x == goal_x && y == goal_y && wc == L'■') return 1;

    // Sinon, seulement les espaces vides, sortie et borne
    return (wc == L' ' || wc == L'►' || wc == L'◄' || wc == L'S' || wc == L'B');
}

// Vérifier si une position est occupée par une autre voiture
int position_occupee(int x, int y, VEHICULE* liste, VEHICULE* ignore) {
    VEHICULE* v = liste;
    while (v) {
        // Ignorer la voiture qu'on est en train de déplacer
        if (v != ignore && v->posx == x && v->posy == y) {
            return 1;  // Position occupée
        }
        v = v->NXT;
    }
    return 0;  // Position libre
}

// =======================================
// Structure pour le chemin pré-calculé
// =======================================
typedef struct {
    int x, y;
} Point;

// =======================================
// Structure pour A* (simplifiée)
// =======================================
typedef struct {
    int x, y;
    int g, h, f;
    int parent_x, parent_y;
} Noeud;

int distance_manhattan(int x1, int y1, int x2, int y2) {
    return abs(x1 - x2) + abs(y1 - y2);
}

// =======================================
// BFS (Breadth-First Search) - Plus simple et robuste
// =======================================
#define MAX_QUEUE 10000

Point* bfs_pathfinding(int start_x, int start_y, int goal_x, int goal_y,
                       wchar_t plan[max_ligne][max_colonne], int* longueur_chemin) {

    static int visited[max_ligne][max_colonne];
    static int parent_x[max_ligne][max_colonne];
    static int parent_y[max_ligne][max_colonne];

    // Réinitialiser
    for (int i = 0; i < max_ligne; i++) {
        for (int j = 0; j < max_colonne; j++) {
            visited[i][j] = 0;
            parent_x[i][j] = -1;
            parent_y[i][j] = -1;
        }
    }

    // Queue pour BFS
    static Point queue[MAX_QUEUE];
    int queue_start = 0;
    int queue_end = 0;

    // Ajouter le point de départ
    queue[queue_end].x = start_x;
    queue[queue_end].y = start_y;
    queue_end++;
    visited[start_x][start_y] = 1;

    int dx[] = {-1, 1, 0, 0};
    int dy[] = {0, 0, -1, 1};

    while (queue_start < queue_end) {
        Point current = queue[queue_start++];

        // Trouvé la destination ?
        if (current.x == goal_x && current.y == goal_y) {
            // Reconstruire le chemin
            Point* chemin = malloc(sizeof(Point) * MAX_QUEUE);
            int idx = 0;

            int cx = goal_x, cy = goal_y;

            while (cx != start_x || cy != start_y) {
                chemin[idx].x = cx;
                chemin[idx].y = cy;
                idx++;

                int px = parent_x[cx][cy];
                int py = parent_y[cx][cy];

                if (px == -1 || py == -1) break;

                cx = px;
                cy = py;
            }

            // Inverser le chemin
            for (int i = 0; i < idx / 2; i++) {
                Point tmp = chemin[i];
                chemin[i] = chemin[idx - 1 - i];
                chemin[idx - 1 - i] = tmp;
            }

            *longueur_chemin = idx;
            return chemin;
        }

        // Explorer les voisins
        for (int i = 0; i < 4; i++) {
            int nx = current.x + dx[i];
            int ny = current.y + dy[i];

            if (nx < 0 || nx >= max_ligne || ny < 0 || ny >= max_colonne) continue;
            if (visited[nx][ny]) continue;

            // Vérifier si accessible
            int accessible = 0;
            if (nx == goal_x && ny == goal_y && plan[nx][ny] == L'■') {
                accessible = 1; // Destination (place de parking)
            } else if (nx == goal_x && ny == goal_y && plan[nx][ny] == L'S') {
                accessible = 1; // Destination (sortie)
            } else if (plan[nx][ny] == L' ' || plan[nx][ny] == L'►' || plan[nx][ny] == L'◄' || plan[nx][ny] == L'S') {
                accessible = 1; // Espace libre
            }

            if (!accessible) continue;

            visited[nx][ny] = 1;
            parent_x[nx][ny] = current.x;
            parent_y[nx][ny] = current.y;

            if (queue_end < MAX_QUEUE) {
                queue[queue_end].x = nx;
                queue[queue_end].y = ny;
                queue_end++;
            }
        }
    }

    *longueur_chemin = 0;
    return NULL;
}

// =======================================
// A* simplifié avec limite d'itérations
// =======================================
#define MAX_ITERATIONS 10000
#define MAX_NOEUDS 5000

Point* astar_simple(int start_x, int start_y, int goal_x, int goal_y,
                    wchar_t plan[max_ligne][max_colonne], int* longueur_chemin) {

    static Noeud noeuds[MAX_NOEUDS];
    int nb_noeuds = 0;
    
    // Noeud de départ
    noeuds[0].x = start_x;
    noeuds[0].y = start_y;
    noeuds[0].g = 0;
    noeuds[0].h = distance_manhattan(start_x, start_y, goal_x, goal_y);
    noeuds[0].f = noeuds[0].h;
    noeuds[0].parent_x = -1;
    noeuds[0].parent_y = -1;
    nb_noeuds = 1;
    
    int fermes[MAX_NOEUDS] = {0};
    
    int dx[] = {-1, 1, 0, 0};
    int dy[] = {0, 0, -1, 1};
    
    int iterations = 0;
    
    while (iterations++ < MAX_ITERATIONS) {
        // Trouver le noeud ouvert avec le plus petit f
        int min_idx = -1;
        int min_f = 999999;
        
        for (int i = 0; i < nb_noeuds; i++) {
            if (!fermes[i] && noeuds[i].f < min_f) {
                min_f = noeuds[i].f;
                min_idx = i;
            }
        }
        
        if (min_idx == -1) break; // Plus de noeuds ouverts
        
        Noeud current = noeuds[min_idx];
        fermes[min_idx] = 1;
        
        // Arrivé au but ?
        if (current.x == goal_x && current.y == goal_y) {
            // Reconstruire le chemin
            Point* chemin = malloc(sizeof(Point) * MAX_NOEUDS);
            int idx = 0;
            
            int cx = goal_x, cy = goal_y;
            
            while ((cx != start_x || cy != start_y) && idx < MAX_NOEUDS) {
                chemin[idx].x = cx;
                chemin[idx].y = cy;
                idx++;
                
                // Trouver le parent
                int trouve = 0;
                for (int i = 0; i < nb_noeuds; i++) {
                    if (noeuds[i].x == cx && noeuds[i].y == cy) {
                        cx = noeuds[i].parent_x;
                        cy = noeuds[i].parent_y;
                        trouve = 1;
                        break;
                    }
                }
                
                if (!trouve) break;
            }
            
            // Inverser le chemin
            for (int i = 0; i < idx / 2; i++) {
                Point tmp = chemin[i];
                chemin[i] = chemin[idx - 1 - i];
                chemin[idx - 1 - i] = tmp;
            }
            
            *longueur_chemin = idx;
            return chemin;
        }
        
        // Explorer les voisins
        for (int i = 0; i < 4; i++) {
            int nx = current.x + dx[i];
            int ny = current.y + dy[i];

            if (!est_accessible(nx, ny, goal_x, goal_y, plan)) continue;
            
            // Vérifier si déjà fermé
            int idx_ferme = -1;
            for (int j = 0; j < nb_noeuds; j++) {
                if (noeuds[j].x == nx && noeuds[j].y == ny) {
                    idx_ferme = j;
                    break;
                }
            }
            
            if (idx_ferme != -1 && fermes[idx_ferme]) continue;
            
            int nouveau_g = current.g + 1;
            
            if (idx_ferme == -1) {
                // Nouveau noeud
                if (nb_noeuds < MAX_NOEUDS) {
                    noeuds[nb_noeuds].x = nx;
                    noeuds[nb_noeuds].y = ny;
                    noeuds[nb_noeuds].g = nouveau_g;
                    noeuds[nb_noeuds].h = distance_manhattan(nx, ny, goal_x, goal_y);
                    noeuds[nb_noeuds].f = nouveau_g + noeuds[nb_noeuds].h;
                    noeuds[nb_noeuds].parent_x = current.x;
                    noeuds[nb_noeuds].parent_y = current.y;
                    nb_noeuds++;
                }
            } else if (nouveau_g < noeuds[idx_ferme].g) {
                noeuds[idx_ferme].g = nouveau_g;
                noeuds[idx_ferme].f = nouveau_g + noeuds[idx_ferme].h;
                noeuds[idx_ferme].parent_x = current.x;
                noeuds[idx_ferme].parent_y = current.y;
            }
        }
    }
    
    *longueur_chemin = 0;
    return NULL;
}

// =======================================
// Déplacer vers la sortie
// =======================================
int deplacer_vers_sortie(VEHICULE* v, wchar_t plan[max_ligne][max_colonne], VEHICULE* liste) {
    if (!v) return 0;

    // Arrivé à la sortie ?
    if (v->posx == SORTIE_X && v->posy == SORTIE_Y) {
        return 1; // Signal pour supprimer la voiture
    }

    // Si à la borne de paiement, attendre 2 secondes
    if (v->posx == BORNE_X && v->posy == BORNE_Y) {
        if (v->ticks_a_borne == -1) {
            // Première arrivée à la borne
            v->ticks_a_borne = 0;
            return 0;  // Ne pas bouger ce tick
        } else if (v->ticks_a_borne < TICKS_PAIEMENT) {
            // Continuer à attendre
            v->ticks_a_borne++;
            return 0;  // Ne pas bouger
        }
        // Sinon, le paiement est terminé, continuer vers la sortie
    }

    // Déterminer la destination : borne si pas encore payé, sinon sortie
    int dest_x, dest_y;
    if (v->ticks_a_borne < TICKS_PAIEMENT) {
        // Pas encore payé, aller à la borne
        dest_x = BORNE_X;
        dest_y = BORNE_Y;
    } else {
        // Paiement effectué, aller à la sortie
        dest_x = SORTIE_X;
        dest_y = SORTIE_Y;
    }

    // Calculer le chemin avec BFS vers la destination
    int longueur_chemin = 0;
    Point* chemin = bfs_pathfinding(v->posx, v->posy, dest_x, dest_y, plan, &longueur_chemin);

    // Si un chemin existe, suivre le premier pas
    if (chemin && longueur_chemin > 0) {
        // Vérifier s'il n'y a pas de collision
        if (!position_occupee(chemin[0].x, chemin[0].y, liste, v)) {
            int old_x = v->posx;
            int old_y = v->posy;
            v->posx = chemin[0].x;
            v->posy = chemin[0].y;

            // Mettre à jour la direction selon le mouvement
            if (chemin[0].x < old_x) v->direction = 'N';      // Haut
            else if (chemin[0].x > old_x) v->direction = 'S'; // Bas
            else if (chemin[0].y < old_y) v->direction = 'W'; // Gauche
            else if (chemin[0].y > old_y) v->direction = 'E'; // Droite
        }
        // Sinon, on attend (ne bouge pas ce tick)
        free(chemin);
        return 0;
    }

    // Fallback : se rapprocher de la destination
    int dx[] = {-1, 1, 0, 0};
    int dy[] = {0, 0, -1, 1};

    int meilleur_x = v->posx;
    int meilleur_y = v->posy;
    int meilleure_dist = distance_manhattan(v->posx, v->posy, dest_x, dest_y);

    for (int i = 0; i < 4; i++) {
        int nx = v->posx + dx[i];
        int ny = v->posy + dy[i];

        // Vérifier que l'espace est praticable ET non occupé
        if ((est_un_espace(nx, ny, plan) || (nx == dest_x && ny == dest_y))
            && !position_occupee(nx, ny, liste, v)) {
            int dist = distance_manhattan(nx, ny, dest_x, dest_y);
            if (dist < meilleure_dist) {
                meilleure_dist = dist;
                meilleur_x = nx;
                meilleur_y = ny;
            }
        }
    }

    if (meilleur_x != v->posx || meilleur_y != v->posy) {
        int old_x = v->posx;
        int old_y = v->posy;
        v->posx = meilleur_x;
        v->posy = meilleur_y;

        // Mettre à jour la direction selon le mouvement
        if (meilleur_x < old_x) v->direction = 'N';      // Haut
        else if (meilleur_x > old_x) v->direction = 'S'; // Bas
        else if (meilleur_y < old_y) v->direction = 'W'; // Gauche
        else if (meilleur_y > old_y) v->direction = 'E'; // Droite
    }

    return 0;
}

// =======================================
// Déplacement intelligent qui évite les obstacles
// =======================================


void deplacer_voiture_vers(VEHICULE* v, PLACE* target, wchar_t plan[max_ligne][max_colonne], VEHICULE* liste) {
    if (!v || !target) return;

    // Arrivé à destination (place de parking)
    if (v->posx == target->x && v->posy == target->y) {
        if (v->ticks_gare == -1) {
            // Entre 25 et 75 ticks = 5-15 secondes à 200ms/tick
            v->ticks_gare = 75 + rand() % 300;
            target->libre = 0;
            v->etat = 0;
        }
        return;
    }

    // Calculer le chemin avec BFS (plus robuste que A*)
    int longueur_chemin = 0;
    Point* chemin = bfs_pathfinding(v->posx, v->posy, target->x, target->y, plan, &longueur_chemin);

    // Si un chemin existe, suivre le premier pas
    if (chemin && longueur_chemin > 0) {
        // Vérifier s'il n'y a pas de collision
        if (!position_occupee(chemin[0].x, chemin[0].y, liste, v)) {
            int old_x = v->posx;
            int old_y = v->posy;
            v->posx = chemin[0].x;
            v->posy = chemin[0].y;

            // Mettre à jour la direction selon le mouvement
            if (chemin[0].x < old_x) v->direction = 'N';      // Haut
            else if (chemin[0].x > old_x) v->direction = 'S'; // Bas
            else if (chemin[0].y < old_y) v->direction = 'W'; // Gauche
            else if (chemin[0].y > old_y) v->direction = 'E'; // Droite
        }
        // Sinon, on attend (ne bouge pas ce tick)
        free(chemin);
        return;
    }

    // Si A* échoue, essayer TOUTES les directions et éviter les obstacles
    int dx[] = {-1, 1, 0, 0};
    int dy[] = {0, 0, -1, 1};

    // Essayer d'abord la direction qui rapproche de la cible
    int meilleur_x = v->posx;
    int meilleur_y = v->posy;
    int meilleure_dist = distance_manhattan(v->posx, v->posy, target->x, target->y);

    for (int i = 0; i < 4; i++) {
        int nx = v->posx + dx[i];
        int ny = v->posy + dy[i];

        // Vérifier si c'est un espace valide OU la destination finale
        int est_valide = 0;
        if (nx == target->x && ny == target->y) {
            est_valide = 1; // Autoriser la destination même si c'est une place
        } else {
            est_valide = est_un_espace(nx, ny, plan);
        }

        // Vérifier aussi que la position n'est pas occupée par une autre voiture
        if (est_valide && !position_occupee(nx, ny, liste, v)) {
            int dist = distance_manhattan(nx, ny, target->x, target->y);
            if (dist < meilleure_dist) {
                meilleure_dist = dist;
                meilleur_x = nx;
                meilleur_y = ny;
            }
        }
    }

    // Si on a trouvé une meilleure position, bouger
    if (meilleur_x != v->posx || meilleur_y != v->posy) {
        int old_x = v->posx;
        int old_y = v->posy;
        v->posx = meilleur_x;
        v->posy = meilleur_y;

        // Mettre à jour la direction selon le mouvement
        if (meilleur_x < old_x) v->direction = 'N';      // Haut
        else if (meilleur_x > old_x) v->direction = 'S'; // Bas
        else if (meilleur_y < old_y) v->direction = 'W'; // Gauche
        else if (meilleur_y > old_y) v->direction = 'E'; // Droite
        return;
    }

    // Sinon, essayer n'importe quelle direction libre (pour ne pas rester bloqué)
    for (int i = 0; i < 4; i++) {
        int nx = v->posx + dx[i];
        int ny = v->posy + dy[i];

        if (est_un_espace(nx, ny, plan) && !position_occupee(nx, ny, liste, v)) {
            int old_x = v->posx;
            int old_y = v->posy;
            v->posx = nx;
            v->posy = ny;

            // Mettre à jour la direction selon le mouvement
            if (nx < old_x) v->direction = 'N';      // Haut
            else if (nx > old_x) v->direction = 'S'; // Bas
            else if (ny < old_y) v->direction = 'W'; // Gauche
            else if (ny > old_y) v->direction = 'E'; // Droite
            return;
        }
    }
}