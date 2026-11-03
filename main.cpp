
#include <iostream>
#include <vector>
#include <thread>
#include <string>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>

using namespace std;

sem_t mesas;
pthread_mutex_t mutexLog;

// Iniciar tablero
void inicializarTablero(string tablero[5][5])
{
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            tablero[i][j] = "V";
        }
    }

    tablero[2][2] = "PU";

    // Colocar los 12 perros
    tablero[0][0] = "P";
    tablero[0][1] = "P";
    tablero[0][2] = "P";
    tablero[0][3] = "P";

    tablero[1][0] = "P";
    tablero[1][1] = "P";
    tablero[1][2] = "P";
    tablero[1][3] = "P";

    tablero[2][0] = "P";
    tablero[2][1] = "P";
    tablero[2][3] = "P";
    tablero[2][4] = "P";
}

// Mostrar tablero
void mostrarTablero(string tablero[5][5])
{
    cout << "\n------ TABLERO KOMIKAN------\n";

    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            cout << "[" << tablero[i][j] << "] ";
        }
        cout << endl;
    }

    cout << endl;
}

// Simulacion de una partida
void jugarPartida(int id)
{
    // Esperar mesa disponible
    sem_wait(&mesas);

    pthread_mutex_lock(&mutexLog);

    cout << "\nPartida " << id << " comienza." << endl;

    // Crear tablero de la partida
    string tablero[5][5];

    inicializarTablero(tablero);
    mostrarTablero(tablero);

    pthread_mutex_unlock(&mutexLog);

    sleep(2);

    pthread_mutex_lock(&mutexLog);

    cout << "Partida " << id << " termina." << endl;

    pthread_mutex_unlock(&mutexLog);

    // Liberar mesa
    sem_post(&mesas);
}

int main(int argc, char* argv[])
{
    if (argc != 3) {
        cout << "Uso: ./torneo N M" << endl;
        return 1;
    }

    int N = stoi(argv[1]);
    int M = stoi(argv[2]);

    if (N < 2 || N % 2 != 0 || M < 1) {
        cout << "Parametros invalidos." << endl;
        return 1;
    }

    // Inicializar sincronizacion
    sem_init(&mesas, 0, M);
    pthread_mutex_init(&mutexLog, nullptr);

    vector<thread> partidas;

    int cantidadPartidas = N / 2;

    // Crear partidas
    for (int i = 0; i < cantidadPartidas; i++) {
        partidas.emplace_back(jugarPartida, i + 1);
    }

    // Esperar que terminen
    for (auto& partida : partidas) {
        partida.join();
    }

    // Liberar recursos
    sem_destroy(&mesas);
    pthread_mutex_destroy(&mutexLog);

    cout << "\nTorneo finalizado." << endl;

    return 0;
}
