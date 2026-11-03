#include <iostream>
#include <vector>
#include <thread>
#include <string>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <cstdlib>
#include <ctime>

using namespace std;

// Sincronizacion
sem_t mesas;
pthread_mutex_t mutexLog;

// Conexiones del tablero
vector<vector<int>> conexiones(25);

void conectar(int a, int b)
{
    conexiones[a].push_back(b);
    conexiones[b].push_back(a);
}

void inicializarConexiones()
{
    for (int fila = 0; fila < 5; fila++) {
        for (int col = 0; col < 5; col++) {

            int actual = fila * 5 + col;

            if (col < 4)
                conectar(actual, actual + 1);

            if (fila < 4)
                conectar(actual, actual + 5);
        }
    }
}

// Generar una espera aleatoria entre 50 y 300 ms
int generarEspera()
{
    return 50 + rand() % 251;
}

// Probar los tiempos aleatorios
void probarTiempo()
{
    for (int i = 0; i < 10; i++) {
        int tiempo = generarEspera();

        cout << "Espera: " << tiempo << " ms" << endl;
    }
}

// Inicializar tablero
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

// Mover una ficha
bool moverFicha(string tablero[5][5], int origen, int destino)
{
    // Verificar que las posiciones existan
    if (origen < 0 || origen >= 25 ||
        destino < 0 || destino >= 25) {
        return false;
    }

    int filaOrigen = origen / 5;
    int colOrigen = origen % 5;

    int filaDestino = destino / 5;
    int colDestino = destino % 5;

    // Verificar que el origen tenga una ficha
    if (tablero[filaOrigen][colOrigen] == "V") {
        return false;
    }

    // Verificar que el destino este vacio
    if (tablero[filaDestino][colDestino] != "V") {
        return false;
    }

    // Verificar que exista una conexion
    bool conectado = false;

    for (int posicion : conexiones[origen]) {
        if (posicion == destino) {
            conectado = true;
            break;
        }
    }

    if (!conectado) {
        return false;
    }

    // Realizar movimiento
    tablero[filaDestino][colDestino] = tablero[filaOrigen][colOrigen];
    tablero[filaOrigen][colOrigen] = "V";

    return true;
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

    inicializarConexiones();
    srand(time(nullptr));

    // Probar los tiempos aleatorios
    probarTiempo();

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
