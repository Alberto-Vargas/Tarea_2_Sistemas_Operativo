# Tarea_2_Sistemas_Operativo

README – Torneo Komikan

1. Descripción general

El proyecto consiste en una simulación de un torneo de "Komikan". Los jugadores se conectan al servidor y son agrupados de dos en dos mediante un Matchmaker, asignando a cada uno el control del Puma o de los perros.

Cada partida utiliza un hilo para el Puma y doce hilos para los perros. La cantidad de partidas que pueden ejecutarse simultáneamente depende del número de mesas disponibles.

2. Compilación y ejecución

El programa está desarrollado en C++ y utiliza el estándar C++17, junto con la biblioteca pthread para la gestión de hilos.

Para compilar directamente:

g++ -Wall -Wextra -std=c++17 main.cpp -o torneo -pthread

También se puede utilizar el script :

./torneo N M

Para utilizarlo:
Donde

N: cantidad total de jugadores. Debe ser par y mayor o igual a 2.

M: cantidad de mesas disponibles. Debe ser mayor o igual a 1.

Ejemplo:

./torneo 10 2

El archivo config.txt que esta ubicado en el mismo directorio desde donde se ejecuta el programa.

3. Formato de configuración

El programa utiliza un archivo externo llamado config.txt para establecer los parámetros de la simulación sin modificar el código fuente.

Ejemplo de configuración:

retardo_min=50
retardo_max=300

puma=12

perros=0,1,2,3,5,6,7,8,10,11,13,14

seed=12345

Descripción de los parámetros:

retardo_min: tiempo mínimo de espera antes de intentar realizar un movimiento, expresado en milisegundos.

retardo_max: tiempo máximo de espera antes de intentar realizar un movimiento, expresado en milisegundos.

puma: posición inicial del Puma en el tablero.

perros: posiciones iniciales de los doce perros, separadas por comas.

seed: semilla utilizada para inicializar los generadores de números aleatorios.

Las posiciones deben estar comprendidas entre 0 y 27, no pueden repetirse y deben existir exactamente doce posiciones para los perros. El programa valida los parámetros antes de iniciar el torneo.

4. Decisiones de sincronización

Para coordinar los diferentes hilos y evitar problemas de concurrencia se utilizan los siguientes mecanismos:

Semáforo mesas: controla la cantidad de partidas simultáneas. Se inicializa con el número de mesas disponibles (M). Cada partida solicita acceso mediante sem_wait() y libera el recurso al finalizar mediante sem_post().

Mutex partida.mutex: protege el estado compartido de cada partida, incluyendo el tablero, las posiciones de las fichas, las capturas y el estado de finalización. Evita que varios hilos modifiquen simultáneamente el estado de una misma partida.

Mutex mutexLog: protege los mensajes y la impresión del tablero en consola, evitando que las salidas de distintas partidas se mezclen.

Mutex mutexCola: protege la cola de jugadores compartida entre los hilos que registran las conexiones y el Matchmaker.

Variable de condición nuevosJugadores: permite que el Matchmaker espere hasta que existan al menos dos jugadores disponibles en la cola. Cuando llega un jugador, se utiliza notify_one() para notificar que hay nuevos jugadores.

Estos mecanismos permiten coordinar la ejecución concurrente, proteger los datos compartidos y controlar el acceso a los recursos del torneo.
5. Justificación del diseño 

El diseño se realizó separando las responsabilidades del sistema para permitir que diferentes procesos puedan ejecutarse de manera concurrente sin interferir incorrectamente entre ellos.

Se decidió utilizar un hilo independiente por jugador dentro de cada partida porque esto permite representar los movimientos del Puma y de los perros como procesos concurrentes. Cada ficha puede intentar realizar sus movimientos de manera independiente, mientras que los mecanismos de sincronización garantizan que el tablero mantenga un estado consistente.

También se decidió utilizar un Matchmaker separado de las partidas para desacoplar la recepción de jugadores de la ejecución de los enfrentamientos. De esta forma, el servidor puede continuar recibiendo y organizando jugadores mientras las partidas anteriores siguen ejecutándose.

La utilización de un semáforo para administrar las mesas permite controlar los recursos limitados del torneo. Aunque existan varias partidas pendientes, solamente se ejecutará simultáneamente la cantidad permitida por el número de mesas.

Finalmente, se optó por utilizar un mutex independiente para cada partida en lugar de uno global para todos los tableros. Esto permite que distintas partidas avancen simultáneamente sin bloquearse entre sí por modificaciones que pertenecen a tableros diferentes.
También se utilizo la estructura de 5 x 5 mas el triangulo que se pedía en el enunciado. 

