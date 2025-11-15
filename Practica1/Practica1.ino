#include <Controllino.h> // Librería necesaria para manejar los pines del Controllino

// ===== Temporización no bloqueante =====
// Variables para implementar el retardo sin detener el programa
unsigned long t_previo = 0;     // Guarda el tiempo previo
unsigned long t_actual = 0;     // Guarda el tiempo actual
unsigned long intervalo = 500;  // Intervalo entre cambios (500 ms)

// Índice para recorrer el arreglo de LEDs
int i = 0;

// Asignación de pines del Controllino a variables para mejor lectura
int D0 = CONTROLLINO_D0;
int D1 = CONTROLLINO_D1;
int D2 = CONTROLLINO_D2;
int D6 = CONTROLLINO_D6;
int D7 = CONTROLLINO_D7;
int D8 = CONTROLLINO_D8;
int D12 = CONTROLLINO_D12;
int D13 = CONTROLLINO_D13;
int D14 = CONTROLLINO_D14;

// Arreglo con el orden en el que quiero encender los LEDs del tablero
int leds[9] = {D0, D6, D12, D13, D14, D8, D2, D1, D7};

// Puntero que apunta al arreglo de LEDs
int* ptr = leds;

void setup() {
  // Configuro todos los pines como salidas digitales
  pinMode(CONTROLLINO_D0, OUTPUT);
  pinMode(CONTROLLINO_D1, OUTPUT);
  pinMode(CONTROLLINO_D2, OUTPUT);
  pinMode(CONTROLLINO_D6, OUTPUT);
  pinMode(CONTROLLINO_D7, OUTPUT);
  pinMode(CONTROLLINO_D8, OUTPUT);
  pinMode(CONTROLLINO_D12, OUTPUT);
  pinMode(CONTROLLINO_D13, OUTPUT);
  pinMode(CONTROLLINO_D14, OUTPUT);
}

void loop() {
  
  // Leo el tiempo actual del sistema
  t_actual = millis();

  // Enciendo el LED correspondiente según el índice i,
  // usando aritmética de punteros: *(ptr + i) es leds[i]
  digitalWrite(*(ptr + i), HIGH);

  // Verifico si ya pasó el intervalo definido
  if (t_actual - t_previo >= intervalo) {

    // Apago el LED actual antes de avanzar al siguiente
    digitalWrite(*(ptr + i), LOW);

    // Actualizo el tiempo previo
    t_previo = t_actual;

    // Avanzo al siguiente LED
    i += 1;

    // Si llego al final del arreglo, regreso al inicio
    if (i == 9){
      i = 0;
    }
  }
}
