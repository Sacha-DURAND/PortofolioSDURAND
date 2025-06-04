/**
 * @file < version3 >
 * 
 * @brief < Programme Snake >
 * 
 * @author < Durand Nathan et Gout Adrien >
 * 
 * @date < 13/01/2025 >
 * 
 * < Programme de déplacement d'un serpennt de taille 10 
 *   qui se dirige automatiquement en evitant les pavé a la prochaine pomme 
 *   en prenant un portail si cela est rentable le serpent 
 *   ne peut pas se croiser ni faire demi tour le programe 
 *   s'arrete si on appuie sur a ou quand on mange 10 pommes
 *   quand le programme s'arrete il affiche le temps cpu et 
 *   les deplacement unitaires>
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <time.h>
#include <math.h>

#define TAILLE 10
// dimensions du plateau
#define LARGEUR_PLATEAU 80  
#define HAUTEUR_PLATEAU 40  //  
// position initiale de la tête du serpent
#define X_INITIAL 40
#define Y_INITIAL 20
// nombre de pommes à manger pour gagner
#define NB_POMMES 10
// temporisation entre deux déplacements du serpent (en microsecondes)
#define ATTENTE 20000
#define INT_MAX 2147483647
// caractères pour représenter le serpent
#define CORPS 'X'
#define TETE 'O'
// touches de direction ou d'arrêt du jeu
#define HAUT 'z'
#define BAS 's'
#define GAUCHE 'q'
#define DROITE 'd'
#define STOP 'a'
// caractères pour les éléments du plateau
#define BORDURE '#'
#define VIDE ' '
#define POMME '6'
#define PAVE 'M'
// nombre et taille des pavés
#define NB_PAVES 6
#define TAILLE_PAVES 5
// définition d'un type pour le plateau : tPlateau
// Attention, pour que les indices du tableau 2D (qui commencent à 0) coincident
// avec les coordonées à l'écran (qui commencent à 1), on ajoute 1 aux dimensions
// et on neutralise la ligne 0 et la colonne 0 du tableau 2D (elles ne sont jamais
// utilisées)
typedef char tPlateau[LARGEUR_PLATEAU+1][HAUTEUR_PLATEAU+1];
typedef int tPommes[NB_POMMES];
typedef int tPaves[NB_PAVES];
// Structure pour représenter une position
typedef struct {
    int x, y;
} Position;

// Fonction pour calculer la distance de Manhattan
int distanceManhattan(Position a, Position b) {
    return abs(a.x - b.x) + abs(a.y - b.y);
}

void initPlateau(tPlateau plateau, tPaves lesPavesX, tPaves lesPavesY);
void dessinerPlateau(tPlateau plateau);
void ajouterPomme(tPlateau plateau, tPommes lesPommesX, tPommes lesPommesY, int nbPommes);
void afficher(int, int, char);
void effacer(int x, int y);
void dessinerSerpent(int lesX[], int lesY[]);
void progresser(int lesX[], int lesY[], char direction, tPlateau plateau, bool * collision, bool * pomme);
void gotoxy(int x, int y);
char calculDirection(int lesX[], int lesY[], int cibleX, int cibleY, char direction, tPlateau leplateau);
void calculDirectionpave(tPlateau plateau, int lesX[], int lesY[],int cibleX, int cibleY, char *direction);
bool astarPathfinding(char plateau[LARGEUR_PLATEAU+1][HAUTEUR_PLATEAU+1], Position start, Position goal, Position chemin[], int *longueurChemin);
char prochaineDirection(Position chemin[], int longueurChemin, int teteX, int teteY);
int kbhit();
void disable_echo();
void enable_echo();


int main(){
    clock_t begin = clock();
    // 2 tableaux contenant les positions des éléments qui constituent le serpent
    int lesX[TAILLE];
    int lesY[TAILLE];
    // position de la tete et de la pomme
    int cibleX, cibleY;
    tPommes lesPommesX = {75, 75, 78, 2, 8, 78, 74, 2, 72, 5};
    tPommes lesPommesY = { 8, 39, 2, 2, 5, 39, 33, 38, 35, 2};
    // position des pavées
    tPaves lesPavesX = { 3, 74, 3, 74, 38, 38};
    tPaves lesPavesY = { 3, 3, 34, 34, 21, 15};
    // compteur de déplacement unitaire
    int cmp = 0;
    // représente la touche frappée par l'utilisateur : touche de direction ou pour l'arrêt
    char touche;

    //direction courante du serpent (HAUT, BAS, GAUCHE ou DROITE)
    char direction = DROITE;

    // le plateau de jeu
    tPlateau lePlateau;

    bool collision=false;
    bool gagne = false;
    bool pommeMangee = false;

    // compteur de pommes mangées
    int nbPommes = 0;

   
    // initialisation de la position du serpent : positionnement de la
    // tête en (X_INITIAL, Y_INITIAL), puis des anneaux à sa gauche
    for(int i=0 ; i<TAILLE ; i++){
        lesX[i] = X_INITIAL-i;
        lesY[i] = Y_INITIAL;
    }

    

    // mise en place du plateau
    initPlateau(lePlateau, lesPavesX, lesPavesY);
    system("clear");
    dessinerPlateau(lePlateau);


    srand(time(NULL));
    ajouterPomme(lePlateau, lesPommesX, lesPommesY, nbPommes);

    // initialisation : le serpent se dirige vers la DROITE
    dessinerSerpent(lesX, lesY);
    disable_echo();
    direction = DROITE;
    touche = DROITE;

    // boucle de jeu. Arret si touche STOP, si collision avec une bordure ou
    // si toutes les pommes sont mangées avec compteur des déplacement unitaire
    while (touche != STOP && !collision && !gagne){
        cibleX = lesPommesX[nbPommes];
        cibleY = lesPommesY[nbPommes];

        direction = calculDirection(lesX, lesY, cibleX, cibleY, direction, lePlateau);
        cmp++;
 
        progresser(lesX, lesY, direction, lePlateau, &collision, &pommeMangee);
        if (pommeMangee){
            nbPommes++;
            gagne = (nbPommes==NB_POMMES);
            if (!gagne){
                ajouterPomme(lePlateau, lesPommesX, lesPommesY, nbPommes);
                pommeMangee = false;
            }   
            
        }
        if (!gagne){
            if (!collision){
                usleep(ATTENTE);
                if (kbhit()==1){
                    touche = getchar();
                }
            }
        }
    }
    enable_echo();
    gotoxy(1, HAUTEUR_PLATEAU+1);
    // Si l'utilisateur gagne on affiche les déplacement unitaire et le temps CPU
    if (gagne){
        
        clock_t end = clock();
        double tmpsCPU = ((end - begin)*1.0) / CLOCKS_PER_SEC;
        printf( "Temps CPU = %.3f secondes\n",tmpsCPU);
        printf("déplacement unitaire : %d", cmp);
        
    }
    return EXIT_SUCCESS;
}


/************************************************/
/*      FONCTIONS ET PROCEDURES DU JEU          */
/************************************************/
//Modifie la direction pour aller a la pomme le plus rapidement en esquivant les pavé
void calculDirectionPave(tPlateau plateau, int lesX[], int lesY[],int cibleX, int cibleY, char *direction){

    Position chemin[TAILLE * TAILLE];
    int longueurChemin = 0;
    Position start = {lesX[0], lesY[0]};
    Position goal = {cibleX, cibleY}; // Position de la pomme

    if (astarPathfinding(plateau, start, goal, chemin, &longueurChemin)) {
        *direction = prochaineDirection(chemin, longueurChemin, start.x, start.y);
    }
}
// fonction qui retourne si un chemin est trouver
bool astarPathfinding(char plateau[LARGEUR_PLATEAU+1][HAUTEUR_PLATEAU+1], Position start, Position goal, Position chemin[], int *longueurChemin) {
    int ouvert[LARGEUR_PLATEAU+1][HAUTEUR_PLATEAU+1] = {0};
    int ferme[LARGEUR_PLATEAU+1][HAUTEUR_PLATEAU+1] = {0};
    int gScore[LARGEUR_PLATEAU+1][HAUTEUR_PLATEAU+1];
    Position parents[LARGEUR_PLATEAU+1][HAUTEUR_PLATEAU+1];

    // Initialiser les scores
    for (int x = 0; x <= LARGEUR_PLATEAU; x++) {
        for (int y = 0; y <= HAUTEUR_PLATEAU; y++) {
            gScore[x][y] = INT_MAX;
            parents[x][y] = (Position){-1, -1};
        }
    }

    // Initialisation
    gScore[start.x][start.y] = 0;
    ouvert[start.x][start.y] = 1;

    while (true) {
        Position courant = { -1, -1 };
        int meilleurScore = INT_MAX;

        // Trouver le meilleur point dans la liste ouverte
        for (int x = 1; x <= LARGEUR_PLATEAU; x++) {
            for (int y = 1; y <= HAUTEUR_PLATEAU; y++) {
                if (ouvert[x][y] && gScore[x][y] + distanceManhattan((Position){x, y}, goal) < meilleurScore) {
                    meilleurScore = gScore[x][y] + distanceManhattan((Position){x, y}, goal);
                    courant = (Position){x, y};
                }
            }
        }

        if (courant.x == -1) {
            return false; // Pas de chemin trouvé
        }

        // Si l'objectif est atteint
        if (courant.x == goal.x && courant.y == goal.y) {
            *longueurChemin = 0;
            while (courant.x != -1) {
                chemin[(*longueurChemin)++] = courant;
                courant = parents[courant.x][courant.y];
            }
            return true;
        }

        // Déplacer le point courant de la liste ouverte à la liste fermée
        ouvert[courant.x][courant.y] = 0;
        ferme[courant.x][courant.y] = 1;

        // Explorer les voisins
        Position voisins[] = {
            {courant.x - 1, courant.y},
            {courant.x + 1, courant.y},
            {courant.x, courant.y - 1},
            {courant.x, courant.y + 1}
        };

        for (int i = 0; i < 4; i++) {
            Position voisin = voisins[i];
            if (voisin.x < 1 || voisin.x > LARGEUR_PLATEAU || voisin.y < 1 || voisin.y > HAUTEUR_PLATEAU) {
                continue;
            }
            if (plateau[voisin.x][voisin.y] == BORDURE || ferme[voisin.x][voisin.y]) {
                continue;
            }

            int tentativeGScore = gScore[courant.x][courant.y] + 1;

            if (!ouvert[voisin.x][voisin.y] || tentativeGScore < gScore[voisin.x][voisin.y]) {
                parents[voisin.x][voisin.y] = courant;
                gScore[voisin.x][voisin.y] = tentativeGScore;
                ouvert[voisin.x][voisin.y] = 1;
            }
        }
    }
}

// Adapter la direction du serpent selon le chemin calculé
char prochaineDirection(Position chemin[], int longueurChemin, int teteX, int teteY) {
    if (longueurChemin < 2) return ' '; // Pas de chemin valide

    Position prochain = chemin[longueurChemin - 2];

    if (prochain.x > teteX) return 'd';
    if (prochain.x < teteX) return 'q';
    if (prochain.y > teteY) return 's';
    if (prochain.y < teteY) return 'z';

    return ' ';
}
char calculDirection(int lesX[], int lesY[], int cibleX, int cibleY,char direction, tPlateau leplateau){
    bool collisionSerpent = false;
    bool collisionPave = false;
    int teteX = lesX[0];
    int teteY = lesY[0];
    int newX = lesX[0];
    int newY = lesY[0];
    // intitialisation de la direction a retrourner
    char res = DROITE;

        if((teteY < cibleY && teteX != LARGEUR_PLATEAU && teteX != 1) || teteY == 1){
            // changement de direction si il y a demi tour
            if (direction != HAUT){res = BAS;}
            else if (teteX + 1 == LARGEUR_PLATEAU){res = GAUCHE;}
            else {res = DROITE;}
        }
        else if((teteY > cibleY && teteX != LARGEUR_PLATEAU && teteX != 1) || teteY == HAUTEUR_PLATEAU){
            // changement de direction si il y a demi tour
            if (direction != BAS){res = HAUT;}
            else if (teteX + 1 == LARGEUR_PLATEAU){res = GAUCHE;}
            else {res = DROITE;}
        }
        else if((teteX < cibleX && teteY != HAUTEUR_PLATEAU && teteY != 1) || teteX == 1){
            // changement de direction si il y a demi tour
            if (direction != GAUCHE){res = DROITE;}
            else if (teteY + 1 == HAUTEUR_PLATEAU) {res = HAUT;}
            else {res = BAS;}
        }
        else if((teteX > cibleX && teteY != HAUTEUR_PLATEAU && teteY != 1) || teteX == LARGEUR_PLATEAU){
            // changement de direction si il y a demi tour
            if (direction != DROITE){res = GAUCHE;}
            else if (teteY + 1 == HAUTEUR_PLATEAU) {res = HAUT;}
            else {res = BAS;}
        }

    // calcul la prochaine position de la tete en fonction de la direction calculé
    if (res == HAUT){newY = teteY - 1;}
    else if (res == BAS){newY = teteY + 1;}
    else if (res == GAUCHE){newX = teteX - 1;}
    else if (res == DROITE){newX = teteX + 1;}
    
    // vérifie si la prochaine position de la tete entre en collision avec le corp du serpent
    for (int i = 0; i < TAILLE; i++) {
        if ( (lesX[i] == newX && lesY[i] == newY)) {collisionSerpent = true;}
    }
    // vérifie si la prochaine position de la tete entre en collision avec un pavé
    if (leplateau[newX][newY] == BORDURE){collisionPave = true;}
     // changement de direction en cas de collision avec le serpent 
    if (collisionSerpent == true){
         if (res == HAUT && lesX[8] < lesX[9]){res = DROITE;}
         else if (res == HAUT && lesX[8] > lesX[9]){res = GAUCHE;}
         else if (res == BAS && lesX[8] < lesX[9]){res = DROITE;}
         else if (res == BAS && lesX[8] > lesX[9]){res = GAUCHE;}
         else if (res == DROITE && lesY[8] < lesY[9]){res = BAS;}
         else if (res == DROITE && lesY[8] > lesY[9]){res = HAUT;}
         else if (res == GAUCHE && lesY[8] < lesY[9]){res = BAS;}
         else if (res == GAUCHE && lesY[8] > lesY[9]){res = HAUT;}
    }
     // changement de direction en cas de collision avec un pavé
    if (collisionPave == true){
        calculDirectionPave(leplateau, lesX, lesY, cibleX, cibleY, &direction);
        res = direction;
    }
    return res;
}
void initPlateau(tPlateau plateau, tPaves lesPavesX, tPaves lesPavesY){
    // initialisation du plateau avec des espaces
    for (int i=1 ; i<=LARGEUR_PLATEAU; i++){
        for (int j=1 ; j<=HAUTEUR_PLATEAU; j++){
            plateau[i][j] = VIDE;
        }
    }
    // Mise en place la bordure autour du plateau
    // première ligne
    for (int i=1 ; i<=LARGEUR_PLATEAU ; i++){
        plateau[i][1] = BORDURE;
    }
    // lignes intermédiaires
    for (int j=1 ; j<=HAUTEUR_PLATEAU; j++){
            plateau[1][j] = BORDURE;
            plateau[LARGEUR_PLATEAU][j] = BORDURE;
            
        }
    // dernière ligne
    for (int i=1 ; i<=LARGEUR_PLATEAU; i++){
        plateau[i][HAUTEUR_PLATEAU] = BORDURE;
        
    }
    // pavée
    for (int i=0; i < NB_PAVES; i++){
        for (int dy = 0; dy < TAILLE_PAVES; dy++) {
            for (int dx = 0; dx < TAILLE_PAVES; dx++) {
                plateau[lesPavesX[i] + dx][lesPavesY[i] + dy] = PAVE;
            }
        }
    }
}

void dessinerPlateau(tPlateau plateau){
    // affiche eà l'écran le contenu du tableau 2D représentant le plateau
    for (int i=1 ; i<=LARGEUR_PLATEAU ; i++){
        for (int j=1 ; j<=HAUTEUR_PLATEAU ; j++){
            afficher(i, j, plateau[i][j]);
        }
    }
    
}



void ajouterPomme(tPlateau plateau, tPommes lesPommesX, tPommes lesPommesY, int nbPommes){
    // génère aléatoirement la position d'une pomme,
    // vérifie que ça correspond à une case vide
    // du plateau puis l'ajoute au plateau et l'affiche
    plateau[lesPommesX[nbPommes]][lesPommesY[nbPommes]]=POMME;
    afficher(lesPommesX[nbPommes], lesPommesY[nbPommes], POMME);
}

void afficher(int x, int y, char car){
    gotoxy(x, y);
    printf("%c", car);
    gotoxy(1,1);
}

void effacer(int x, int y){
    gotoxy(x, y);
    printf(" ");
    gotoxy(1,1);
}

void dessinerSerpent(int lesX[], int lesY[]){
    // affiche les anneaux puis la tête
    for(int i=1 ; i<TAILLE ; i++){
        afficher(lesX[i], lesY[i], CORPS);
    }
    afficher(lesX[0], lesY[0],TETE);
}

void progresser(int lesX[], int lesY[], char direction, tPlateau plateau, bool * collision, bool * pomme){
    // efface le dernier élément avant d'actualiser la position de tous les 
    // élémentds du serpent avant de le  redessiner et détecte une
    // collision avec une pomme ou avec une bordure
    effacer(lesX[TAILLE-1], lesY[TAILLE-1]);

    for(int i=TAILLE-1 ; i>0 ; i--){
        lesX[i] = lesX[i-1];
        lesY[i] = lesY[i-1];
    }
    //faire progresser la tete dans la nouvelle direction
    
    switch(direction){
        case HAUT : 
            lesY[0] = lesY[0] - 1;
            
            break;
        case BAS:
            lesY[0] = lesY[0] + 1;
            
            break;
        case DROITE:
            lesX[0] = lesX[0] + 1;
            
            break;
        case GAUCHE:
            lesX[0] = lesX[0] - 1;
            
            break;
    }
    
    //Si le serpent entre dans un portail
    if ((lesX[0]) == 0 && (plateau[lesY[0]][lesX[0]] != BORDURE)) { // Portail gauche
        lesX[0] = LARGEUR_PLATEAU;
    }
    else if ((lesX[0]) == LARGEUR_PLATEAU && lesY[0] == HAUTEUR_PLATEAU / 2) { //portail droit
        lesX[0] = 1;
    }
    else if ((lesY[0]) == 1 && (lesX[0]) == LARGEUR_PLATEAU / 2) { // portail haut
        lesY[0] = HAUTEUR_PLATEAU;
    }
    else if ((lesY[0]) == HAUTEUR_PLATEAU && (plateau[lesY[0]][lesX[0]] != BORDURE)) { // portail bas
        lesY[0] = 1;
    }
    *pomme = false;
    // détection d'une "collision" avec une pomme
    if (plateau[lesX[0]][lesY[0]] == POMME){
        *pomme = true;
        
        // la pomme disparait du plateau
        plateau[lesX[0]][lesY[0]] = VIDE;
    }
    // détection d'une collision avec la bordure
    else if (plateau[lesX[0]][lesY[0]] == BORDURE){
        *collision = true;
    }
    dessinerSerpent(lesX, lesY);
}



/************************************************/
/*               FONCTIONS UTILITAIRES          */
/************************************************/
void gotoxy(int x, int y) { 
    printf("\033[%d;%df", y, x);
}

int kbhit(){
    // la fonction retourne :
    // 1 si un caractere est present
    // 0 si pas de caractere présent
    int unCaractere=0;
    struct termios oldt, newt;
    int ch;
    int oldf;

    // mettre le terminal en mode non bloquant
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
 
    ch = getchar();

    // restaurer le mode du terminal
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);
 
    if(ch != EOF){
        ungetc(ch, stdin);
        unCaractere=1;
    } 
    return unCaractere;
}

// Fonction pour désactiver l'echo
void disable_echo() {
    struct termios tty;

    // Obtenir les attributs du terminal
    if (tcgetattr(STDIN_FILENO, &tty) == -1) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }

    // Désactiver le flag ECHO
    tty.c_lflag &= ~ECHO;

    // Appliquer les nouvelles configurations
    if (tcsetattr(STDIN_FILENO, TCSANOW, &tty) == -1) {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }
}

// Fonction pour réactiver l'echo
void enable_echo() {
    struct termios tty;

    // Obtenir les attributs du terminal
    if (tcgetattr(STDIN_FILENO, &tty) == -1) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }

    // Réactiver le flag ECHO
    tty.c_lflag |= ECHO;

    // Appliquer les nouvelles configurations
    if (tcsetattr(STDIN_FILENO, TCSANOW, &tty) == -1) {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }
}
// taille du serpent

