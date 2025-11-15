t
#include <Controllino.h>
#include "Stone_HMI_Define.h"
#include "Procesar_HMI.h"

// ===== PID incremental con Kp, Ti, Td =====
volatile float u_actual = 0.0f;  // PWM 0..255
volatile float du = 0.0f;    // Δu (solo monitoreo)
volatile float Kp = 0.4567f;     // Ganancia proporcional
volatile float Ti = 0.09167f;      // Tiempo integral [s]
volatile float Td = 0.0f;       // Tiempo derivativo [s]
const float T = 0.05f;             // Periodo de muestreo [s]
float e[3] = {0,0,0};             // e[k], e[k-1], e[k-2]

// ===== Motor / HMI =====
const int pin_motor = CONTROLLINO_D0;   // Asegúrate que es PWM en tu modelo
volatile int ref = 0;                 // Setpoint
char label_ref[10];
char label_rpm[10];

// ===== Encoder / RPM =====
const int entrada = CONTROLLINO_IN1;
volatile unsigned long conteo_pulsos = 0;
volatile float rpm = 0.0f;
const uint16_t PULSOS_POR_REV = 36;
const float fs = 1.0f / T;

// ===== Tiempos HMI =====
unsigned long t_previo = 0;
unsigned long t_previo1 = 0;

// ===== Prototipo =====
void contarPulso();

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200);

  pinMode(entrada, INPUT);
  pinMode(pin_motor, OUTPUT);

  // ===== HMI =====
  Stone_HMI_Set_Value("slider", "slider1", NULL, 0);
  STONE_push_series("line_series", "line_series1", 0);
  STONE_push_series("line_series", "line_series2", 0);
  STONE_push_series("line_series", "line_series3", 0);   

  // spin_box1=Kp, spin_box2=Ti [s], spin_box3=Td [s]
  Stone_HMI_Set_Value("spin_box", "spin_box1", NULL, 0.4567);
  Stone_HMI_Set_Value("spin_box", "spin_box2", NULL, 0.09167);
  Stone_HMI_Set_Value("spin_box", "spin_box3", NULL, 0.0);

  // Encoder
  attachInterrupt(digitalPinToInterrupt(entrada), contarPulso, FALLING);

  // ===== Timer1: 50 ms =====
  noInterrupts();
  TCCR1A = 0;
  TCCR1B = 0;
  TCCR1B |= B00000100;     // prescaler /256  → 16 µs/tick
  TIMSK1 |= B00000010;     // enable compare A
  OCR1A  = 3125;           // 62500/fs = 62500*0.05  
  interrupts();

  HMI_init();
}

void loop() {
  // Lectura HMI cada ~10 ms
  if (millis() - t_previo1 >= 10) {
    // Lee UI a variables locales primero
    float kp_ui = HMI_get_value("spin_box", "spin_box1");
    float ti_ui = HMI_get_value("spin_box", "spin_box2");
    float td_ui = HMI_get_value("spin_box", "spin_box3");
    int   ref_ui = HMI_get_value("slider",   "slider1");

    // Protege la actualización (sección crítica)
    noInterrupts();
    Kp  = kp_ui;
    Ti  = ti_ui;   // si el spin box puede dar 0, el ISR se protegerá
    Td  = td_ui;
    ref = ref_ui;
    interrupts();

    t_previo1 = millis();
  }

  // Envío HMI cada ~100 ms
  if (millis() - t_previo >= 100) {
    t_previo = millis();
    // Copias locales para imprimir sin jitter
    float rpm_local; int ref_local; float u_local;
    noInterrupts();
    rpm_local = rpm; ref_local = ref; u_local = u_actual;
    interrupts();

    dtostrf((float)ref_local, 7, 2, label_ref);
    dtostrf(rpm_local,        7, 2, label_rpm);

    Stone_HMI_Set_Text("label","label2",label_ref);   // referencia
    Stone_HMI_Set_Text("label","label4",label_rpm);   // rpm

    STONE_push_series("line_series", "line_series1", ref_local);
    STONE_push_series("line_series", "line_series2", rpm_local);
    STONE_push_series("line_series", "line_series3", (int)u_actual);
  }
}

// ===== ISR: muestreo y PID incremental =====
ISR(TIMER1_COMPA_vect) {
  // Si no usas CTC real, resetea contador:
  TCNT1 = 0;

  // RPM en la ventana T
  rpm = ( (float)conteo_pulsos * 60.0f * fs ) / (float)PULSOS_POR_REV;

  // Desplazar errores
  e[2] = e[1];
  e[1] = e[0];
  e[0] = (float)ref - rpm;

  // Copias locales (evita leer compartidos varias veces)
  float Kp_l = Kp;
  float Ti_l = Ti;
  float Td_l = Td;

  // deltau = Kp[(e0−e1) + (T/Ti)e0 + (Td/T)(e0−2e1+e2)] ecuacion de diferencias
  float du_loc = 0.0f;

  // Se emplean condicionales para evitar valores infinitos en salida de PID
  // Se calcula por proporcional, integral, derivativo paso a paso
  du_loc += Kp_l * (e[0] - e[1]);                           // calculo proporcional
  if (Ti_l > 0.0f) du_loc += Kp_l * (T / Ti_l) * e[0];      // calculo integral (solo si Ti>0)
  if (Td_l > 0.0f) du_loc += Kp_l * (Td_l / T) * (e[0] - 2.0f*e[1] + e[2]); // calculo derivativo (solo si Ti>0)

  // Acumular y limitar la salida del PID
  float u_test = u_actual + du_loc;
  if (u_test > 255.0f) u_test = 255.0f;
  if (u_test <   0.0f) u_test =   0.0f;

  u_actual = u_test;
  du = du_loc;

  Serial.println(Kp);
  Serial.println(Ti);
  Serial.println(Td);
  
  analogWrite(pin_motor, (int)u_actual);

  // Mapeo de 0–5 V a 0–255 (PWM)
  //int pwm_out = map((int)(u_actual * 100), 0, 500, 0, 255);
  //analogWrite(pin_motor, pwm_out);

  
  conteo_pulsos = 0;  // Reinicia conteo para la próxima ventana T
}

// ===== ISR de pulsos del encoder =====
void contarPulso() {
  conteo_pulsos++;
}
