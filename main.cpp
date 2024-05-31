#include "lirecourant.h"
#include <iostream> // Inclure iostream pour std::cout et std::endl

int main() {
    // Créer une instance de la classe LecteurCapteurs
    LecteurCapteurs lecteurCapteurs;

    // Effectuer les mesures pendant 10 secondes avec un intervalle de 1 seconde entre chaque mesure
    lecteurCapteurs.mesurerTemps(30, 1);

    // Indiquer que le programme s'est terminé correctement
    std::cout << "Programme terminé avec succès." << std::endl;

    return 0;
}
