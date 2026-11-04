
#include <iostream>
#include <vector>
#include <thread>
#include <string>
#include <semaphore.h>
#include <pthread.h>
#include <random>
#include <chrono>

using namespace std;

// Sincronizacion general
sem_t mesas;
pthread_mutex_t mutexLog;

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

// Conectar dos posiciones
void conectar(int a, int b)
{
    conexiones[a].push_back(b);
    conexiones[b].push_back(a);
}

// Iniciar conexiones
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
    // Trono
 conectar(25, 26);
 conectar(25, 27);
conectar(26, 27);

conectar(26, 0);
conectar(26, 1);

 conectar(27, 3);
 conectar(27, 4);
}

// Inicializar tablero
void inicializarTablero(Partida& partida)
{
    partida.tablero.assign(28, "V");

 partida.posicionPuma = 12;
   partida.tablero[12] = "PU";

    int posicionesIniciales[] = {
        0, 1, 2, 3, 5, 6,
        7, 8, 10, 11, 13, 14
    };

   partida.posicionesPerros.clear();
    
    partida.perrosActivos.assign(12, true);

    for (int i = 0; i < 12; i++) {
        int posicion = posicionesIniciales[i];

    partida.tablero[posicion] = "P";
    partida.posicionesPerros.push_back(posicion);
    }

    partida.capturas = 0;
    partida.terminada = false;
}

// Mostrar tablero
void mostrarTablero(const vector<string>& tablero)
{
    cout << "\n====== TABLERO KOMIKAN ======\n\n";

    cout << "             [" << tablero[25] << "]\n";
    cout << "          [" << tablero[26] << "]   ["
         << tablero[27] << "]\n\n";

    for (int fila = 0; fila < 5; fila++) {
        for (int col = 0; col < 5; col++) {
            cout << "[" << tablero[fila * 5 + col] << "] ";
        }
        cout << endl;
    }

    cout << endl;
}

// Comprobar si dos posiciones estan conectadas
bool estanConectadas(int origen, int destino)
{
    for (int posicion : conexiones[origen]) {
        if (posicion == destino)
            return true;
    }

    return false;
}

// Mover ficha a una posicion adyacente libre
bool moverFicha(vector<string>& tablero, int origen, int destino)
{
    if (origen < 0 || origen >= 28 ||
        destino < 0 || destino >= 28)
        return false;

    if (tablero[origen] == "V")
        return false;

    if (tablero[destino] != "V")
        return false;

    if (!estanConectadas(origen, destino))
        return false;

    tablero[destino] = tablero[origen];
    tablero[origen] = "V";
    return true;
}

// Comprobar si el Puma tiene algun movimiento valido
bool tieneMovimientoPuma(const Partida& partida)
{
    int puma = partida.posicionPuma;

    // Movimientos normales
    for (int destino : conexiones[puma]) {
        if (partida.tablero[destino] == "V")
            return true;
    }

    // Capturas posibles
    for (int perro : conexiones[puma]) {
        if (partida.tablero[perro] == "P") {
            for (int destino : conexiones[perro]) {
                if (partida.tablero[destino] == "V")
                    return true;
         }
     }
  }

    return false;
}

// Comprobar condiciones de victoria
int verificarVictoria(const Partida& partida)
{
    if (partida.capturas >= 6)
        return 1;

    if (!tieneMovimientoPuma(partida))
        return 2;

    return 0;
}

// Imprimir movimiento y tablero
void registrarMovimiento(Partida& partida, const string& mensaje)
{
pthread_mutex_lock(&mutexLog);

 cout << mensaje << endl;
 mostrarTablero(partida.tablero);

  pthread_mutex_unlock(&mutexLog);
}

// Hilo del Puma
void moverPuma(Partida* partida)
{
    random_device rd;
    mt19937 generador(rd());
    uniform_int_distribution<int> espera(50, 300);

    while (true) {
        this_thread::sleep_for(
            chrono::milliseconds(espera(generador))
        );

        pthread_mutex_lock(&partida->mutex);

        if (partida->terminada) {
            pthread_mutex_unlock(&partida->mutex);
            return;
        }

        // Cada opcion contiene destino y perro a capturar.
        // -1 significa movimiento normal.
        vector<pair<int, int>> opciones;

        int origen = partida->posicionPuma;

        // Movimientos normales
        for (int destino : conexiones[origen]) {
            if (partida->tablero[destino] == "V") {
                opciones.push_back({destino, -1});
            }
        }

        // Capturas
        for (int perro : conexiones[origen]) {
            if (partida->tablero[perro] == "P") {
                for (int destino : conexiones[perro]) {
                    if (partida->tablero[destino] == "V") {
                        opciones.push_back({destino, perro});
                    }
                }
            }
        }

        if (opciones.empty()) {
            partida->terminada = true;

            pthread_mutex_unlock(&partida->mutex);

            pthread_mutex_lock(&mutexLog);
            cout << "¡Ganan los perros! El Puma no puede moverse."
                 << endl;
            pthread_mutex_unlock(&mutexLog);

            return;
        }

        uniform_int_distribution<int> elegir(
            0, static_cast<int>(opciones.size()) - 1
        );

        auto movimiento = opciones[elegir(generador)];

        int destino = movimiento.first;
        int perroCapturado = movimiento.second;

        partida->tablero[origen] = "V";

        if (perroCapturado != -1) {
            partida->tablero[perroCapturado] = "V";
            partida->capturas++;

            // Desactivar el hilo del perro capturado
            for (int i = 0; i < 12; i++) {
                if (partida->posicionesPerros[i] == perroCapturado) {
                    partida->perrosActivos[i] = false;
                    partida->posicionesPerros[i] = -1;
                    break;
                }
            }

            cout << "";
        }

        partida->tablero[destino] = "PU";
        partida->posicionPuma = destino;

        bool captura = (perroCapturado != -1);

        int ganador = verificarVictoria(*partida);

        if (ganador != 0)
            partida->terminada = true;

        string mensaje;

        if (captura) {
            mensaje = "El Puma capturo un perro. Total: "
                    + to_string(partida->capturas);
        } else {
            mensaje = "El Puma realizo un movimiento.";
        }

        if (ganador == 1)
            mensaje += "\n¡GANA EL PUMA!";
        else if (ganador == 2)
            mensaje += "\n¡GANAN LOS PERROS!";

        registrarMovimiento(*partida, mensaje);

        pthread_mutex_unlock(&partida->mutex);

        if (ganador != 0)
            return;
    }
}

// Hilo de cada perro
void moverPerro(Partida* partida, int id)
{
    random_device rd;
    mt19937 generador(rd());
    uniform_int_distribution<int> espera(50, 300);

    while (true) {
        this_thread::sleep_for(
            chrono::milliseconds(espera(generador))
        );

        pthread_mutex_lock(&partida->mutex);

        if (partida->terminada || !partida->perrosActivos[id]) {
            pthread_mutex_unlock(&partida->mutex);
            return;
        }

        int origen = partida->posicionesPerros[id];

        vector<int> opciones;

        // Buscar posiciones vacias y conectadas
        for (int destino : conexiones[origen]) {
            if (partida->tablero[destino] == "V") {
                opciones.push_back(destino);
            }
        }

        // Si no puede moverse, espera otro ciclo
        if (!opciones.empty()) {
            uniform_int_distribution<int> elegir(
                0, static_cast<int>(opciones.size()) - 1
            );

            int destino = opciones[elegir(generador)];

            partida->tablero[origen] = "V";
            partida->tablero[destino] = "P";

            partida->posicionesPerros[id] = destino;

            int ganador = verificarVictoria(*partida);

            if (ganador != 0)
                partida->terminada = true;

            string mensaje = "Un perro realizo un movimiento.";

            if (ganador == 1)
                mensaje += "\n¡GANA EL PUMA!";
            else if (ganador == 2)
                mensaje += "\n¡GANAN LOS PERROS!";

            registrarMovimiento(*partida, mensaje);

            pthread_mutex_unlock(&partida->mutex);

            if (ganador != 0)
                return;
        } else {
            pthread_mutex_unlock(&partida->mutex);
        }
    }
}

// Simulacion de una partida
void jugarPartida(int id)
{
    sem_wait(&mesas);

    Partida partida;

    pthread_mutex_init(&partida.mutex, nullptr);
    inicializarTablero(partida);

    pthread_mutex_lock(&mutexLog);
    cout << "\nPartida " << id << " comienza." << endl;
    mostrarTablero(partida.tablero);
    pthread_mutex_unlock(&mutexLog);

    // Un hilo para el Puma
    thread hiloPuma(moverPuma, &partida);

    // Doce hilos para los perros
    vector<thread> hilosPerros;

    for (int i = 0; i < 12; i++) {
        hilosPerros.emplace_back(moverPerro, &partida, i);
    }

    // Esperar a que termine el Puma
    hiloPuma.join();

    // Esperar a que terminen los perros
    for (auto& hilo : hilosPerros) {
        hilo.join();
    }

    pthread_mutex_lock(&mutexLog);
    cout << "\nPartida " << id << " terminada." << endl;
    pthread_mutex_unlock(&mutexLog);

    pthread_mutex_destroy(&partida.mutex);

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

    sem_init(&mesas, 0, M);
    pthread_mutex_init(&mutexLog, nullptr);

    vector<thread> partidas;

    int cantidadPartidas = N / 2;

    for (int i = 0; i < cantidadPartidas; i++) {
        partidas.emplace_back(jugarPartida, i + 1);
    }

    for (auto& partida : partidas) {
        partida.join();
    }

    sem_destroy(&mesas);
    pthread_mutex_destroy(&mutexLog);

    cout << "\nTorneo finalizado." << endl;

    return 0;
}
