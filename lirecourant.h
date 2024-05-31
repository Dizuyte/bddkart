#ifndef LIRECOURANT_H
#define LIRECOURANT_H

class LecteurCapteurs {
public:
    LecteurCapteurs();
    float lireCourantPositif();
    float lireCourantNegatif();
    float lireCourantTotal(); 
    void mesurerTemps(int duree, int intervalle);
    float** getMesures();
    int getNombreMesures();

private:
    int lireMCP3008(int canal);
    int m_spiHandle;
    float** m_mesures;
    int m_nombreMesures;
    int m_intervalle; // Ajouter cette ligne
    void exporterVersBDD(const char* heureDebut, const char* heureFin);
};

#endif // LIRECOURANT_H
