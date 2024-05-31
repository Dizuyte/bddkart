#include <iostream>
#include <sqlite3.h>
#include <fstream>
#include <string>

using namespace std;

// Callback function to process each row of the result set
int callback(void* data, int argc, char** argv, char** azColName) {
    ofstream* outputFile = static_cast<ofstream*>(data);
    
    // Write each column value to the CSV file
    for (int i = 0; i < argc; ++i) {
        *outputFile << argv[i];
        if (i < argc - 1)
            *outputFile << ",";
    }
    *outputFile << endl;

    return 0;
}

void export_database_to_csv(const string& database_name, const string& table_name, string& csv_file) {
    sqlite3* db;
    char* zErrMsg = nullptr;
    int rc;

    // Open database
    rc = sqlite3_open(database_name.c_str(), &db);
    if (rc) {
        cerr << "Can't open database: " << sqlite3_errmsg(db) << endl;
        sqlite3_close(db);
        return;
    }

    // Check if CSV file exists, if yes, create a new file with a numbered suffix
    int count = 0;
    string original_csv_file = csv_file;
    string table_name_suffix = "_" + table_name;
    while (ifstream(csv_file)) {
        ++count;
        csv_file = original_csv_file.substr(0, original_csv_file.size() - 4) + table_name_suffix + to_string(count) + ".csv";
    }

    // Open CSV file for writing
    ofstream outputFile(csv_file);
    if (!outputFile.is_open()) {
        cerr << "Unable to open CSV file for writing." << endl;
        sqlite3_close(db);
        return;
    }

    // SQL query to select all rows from the specified table
    string query = "SELECT * FROM " + table_name;

    // Execute SQL query
    rc = sqlite3_exec(db, query.c_str(), callback, &outputFile, &zErrMsg);
    if (rc != SQLITE_OK) {
        cerr << "SQL error: " << zErrMsg << endl;
        sqlite3_free(zErrMsg);
    }

    // Close database and CSV file
    sqlite3_close(db);
    outputFile.close();

    cout << "Exportation terminée avec succès vers : " << csv_file << endl;
}

int main() {
    // Database name
    string database_name = "exemple.db";

    // Ask the user for the table name to export
    cout << "Entrez le nom de la table à exporter : ";
    string table_name;
    getline(cin, table_name);

    // CSV file name
    string csv_file = "exemple.csv";

    // Export SQLite database to CSV file
    export_database_to_csv(database_name, table_name, csv_file);

    return 0;
}
