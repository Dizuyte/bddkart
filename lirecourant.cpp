#include "lirecourant.h"
#include <iostream>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <cstdlib>
#include <sqlite3.h>
#include <ctime>

#define CANAL_COURANTPOS 1
#define CANAL_COURANTNEG 0
#define VREF 5.0
#define VALEUR_ADC_MAX 1023

LecteurCapteurs::LecteurCapteurs() {
    if (wiringPiSetup() == -1) {
        std::cerr << "Erreur lors de l'initialisation de WiringPi." << std::endl;
        std::exit(1);
    }
    m_spiHandle = wiringPiSPISetup(0, 1000000);
    if (m_spiHandle == -1) {
        std::cerr << "Erreur lors de l'initialisation de la communication SPI." << std::endl;
        std::exit(1);
    }
}

float LecteurCapteurs::lireCourantPositif() {
    int valeurBrute = lireMCP3008(CANAL_COURANTPOS);
    float courant = static_cast<float>(valeurBrute) * (VREF / VALEUR_ADC_MAX);
    return courant;
}

float LecteurCapteurs::lireCourantNegatif() {
    int valeurBrute = lireMCP3008(CANAL_COURANTNEG);
    float courant = static_cast<float>(valeurBrute) * (VREF / VALEUR_ADC_MAX);
    return -courant;
}

float LecteurCapteurs::lireCourantTotal() {
    float courantPositif = lireCourantPositif();
    float courantNegatif = abs(lireCourantNegatif());
    return courantPositif - courantNegatif;
}

void LecteurCapteurs::mesurerTemps(int duree, int intervalle) {
    m_intervalle = intervalle;
    m_nombreMesures = duree / intervalle;
    m_mesures = new float*[m_nombreMesures];

    std::time_t heureDebut = std::time(nullptr);
    char heureDebutStr[20];
    std::strftime(heureDebutStr, sizeof(heureDebutStr), "%Y-%m-%d %H:%M:%S", std::localtime(&heureDebut));

    for (int i = 0; i < m_nombreMesures; ++i) {
        m_mesures[i] = new float[3];
        m_mesures[i][2] = lireCourantTotal(); // Seule la valeur du courant total est stockée
        std::cout << "Mesure " << i << " (temps écoulé) : " << i * intervalle / 60 << ":" << i * intervalle % 60
                  << ", Courant Total = " << m_mesures[i][2] * 100 << " A" << std::endl;
        delay(intervalle * 1000);
    }

    std::time_t heureFin = std::time(nullptr);
    char heureFinStr[20];
    std::strftime(heureFinStr, sizeof(heureFinStr), "%Y-%m-%d %H:%M:%S", std::localtime(&heureFin));

    exporterVersBDD(heureDebutStr, heureFinStr);
}

int LecteurCapteurs::lireMCP3008(int canal) {
    unsigned char buffer[3] = {0};
    buffer[0] = 0x01;
    buffer[1] = 0x80 | (canal << 4);
    buffer[2] = 0x00;
    wiringPiSPIDataRW(0, buffer, 3);
    int resultat = ((buffer[1] & 0x03) << 8) | buffer[2];
    return resultat;
}

float** LecteurCapteurs::getMesures() {
    return m_mesures;
}

int LecteurCapteurs::getNombreMesures() {
    return m_nombreMesures;
}

void LecteurCapteurs::exporterVersBDD(const char* heureDebut, const char* heureFin) {
    sqlite3* db;
    char* zErrMsg = 0;
    int rc;

    rc = sqlite3_open("exemple.db", &db);
    if (rc) {
        std::cerr << "Impossible d'ouvrir la base de données: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    const char* sqlInsertCourse = "INSERT INTO course (num_course, heure_deb_course, heure_fin_course) VALUES (1, ?, ?);";
    sqlite3_stmt* stmt;
    rc = sqlite3_prepare_v2(db, sqlInsertCourse, -1, &stmt, 0);
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, heureDebut, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, heureFin, -1, SQLITE_STATIC);
        rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
    if (rc != SQLITE_DONE) {
        std::cerr << "Erreur lors de l'insertion de la course: " << sqlite3_errmsg(db) << std::endl;
    }

    int idCourse = sqlite3_last_insert_rowid(db);

    const char* sqlInsertMesure = "INSERT INTO mesures_batt (id_mesure, n_mesure, courant, temps) VALUES (?, ?, ?, ?);";
    rc = sqlite3_prepare_v2(db, sqlInsertMesure, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        std::cerr << "Erreur lors de la préparation de l'insertion des mesures: " << sqlite3_errmsg(db) << std::endl;
    }

    for (int i = 0; i < m_nombreMesures; ++i) {
        char tempsMesure[20];
        std::time_t now = std::time(nullptr) + i * m_intervalle;
        std::strftime(tempsMesure, sizeof(tempsMesure), "%Y-%m-%d %H:%M:%S", std::localtime(&now));

        sqlite3_bind_int(stmt, 1, idCourse);
        sqlite3_bind_int(stmt, 2, i);
        sqlite3_bind_double(stmt, 3, m_mesures[i][2] * 100);
        sqlite3_bind_text(stmt, 4, tempsMesure, -1, SQLITE_STATIC);
        
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            std::cerr << "Erreur lors de l'insertion des mesures: " << sqlite3_errmsg(db) << std::endl;
            break;
        }
        sqlite3_reset(stmt);
    }
    sqlite3_finalize(stmt);

    sqlite3_close(db);
    std::cout << "Données exportées avec succès dans la base de données." << std::endl;
}


