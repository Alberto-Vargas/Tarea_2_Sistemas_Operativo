
#include <iostream>
#include <vector>
#include <thread>
#include <string>
#include <semaphore.h>
#include <pthread.h>
#include <random>
#include <chrono>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <fstream>
#include <sstream>
#include <set>

using namespace std;

// Configuracion del torneo
struct Config {
    int retardo_min;
    int retardo_max;
    int puma;
    vector<int> perros;
    unsigned int seed;
};

Config config;

// Sincronizacion general
sem_t mesas;
pthread_mutex_t mutexLog;

// Matchmaker
queue<int> colaJugadores;
mutex mutexCola;
condition_variable nuevosJugadores;

// Conexiones del tablero
vector<vector<int>> conexiones(28);

// Estado de una partida
struct Partida {
    vector<string> tablero;
    vector<int> posicionesPerros;
    vector<bool> perrosActivos;

    int posicionPuma;
    int capturas;
    bool terminada;

    pthread_mutex_t mutex;
};

// Leer configuracion desde archivo
bool leerConfig(string nombreArchivo)
{
    ifstream archivo(nombreArchivo);

    if (!archivo.is_open()) {
        cout << "Error: no se pudo abrir config.txt" << endl;
        return false;
    }

    string linea;

    while (getline(archivo, linea)) {
        if (linea.empty())
            continue;

        stringstream ss(linea);
        string clave, valor;

        getline(ss, clave, '=');
        getline(ss, valor);

        if (clave == "retardo_min")
            config.retardo_min = stoi(valor);
        else if (clave == "retardo_max")
            config.retardo_max = stoi(valor);
        else if (clave == "puma")
            config.puma = stoi(valor);
        else if (clave == "seed")
            config.seed = stoul(valor);
        else if (clave == "perros") {
            stringstream posiciones(valor);
            string numero;

            config.perros.clear();

            while (getline(posiciones, numero, ',')) {
                config.perros.push_back(stoi(numero));
            }
        }
    }

}
