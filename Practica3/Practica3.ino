#include <Controllino.h>
#include "Stone_HMI_Define.h"
#include "Procesar_HMI.h"

const int led              = CONTROLLINO_D0; // Salida digital D0 
const int led2              = CONTROLLINO_D6; // Salida digital D6 
const int boton1            = CONTROLLINO_I16;  // Entrada Digital I16
const int boton2            = CONTROLLINO_I17;  // Entrada Digital I17
int       pwmValue         = 0;              // valor convertido (0-255) led 1
int       pwmValue2         = 0;              // valor convertido (0-255) led 2
float     dutyCyclePercent = 0;              // valor en porcentaje (0-100) led 1
float     dutyCyclePercent2 = 0;              // valor en porcentaje (0-100) led 2
int bandera1                = 0;              // bandera para boton 1
int bandera2                = 0;              // bandera para boton 2
int valor1                  = 0;              // estado del boton 1
int valor2                  = 0;              // estado del boton 2

void setup() {
  Serial.begin(115200);   // Comunicación serial con el PC
  Serial2.begin(115200);  // Comunicación serial con el HMI
  pinMode(led, OUTPUT);   // led1 como salida
  pinMode(led2, OUTPUT);   // led2 como salida
  pinMode(boton1, INPUT);   // boton 1 como entrada
  pinMode(boton2, INPUT);   // boton 2 como entrada
  HMI_init();             // Inicializa el sistema de colas para las respuestas el HMI
  Stone_HMI_Set_Value("spin_box", "sp1", NULL, 0);  // Pone en 0 el valor del spin box en el HMI. 
  Stone_HMI_Set_Value("spin_box", "sp2", NULL, 0);  // Pone en 0 el valor del spin box en el HMI. 
}

void loop() {
  dutyCyclePercent=HMI_get_value("spin_box", "sp1"); // Obtiene el valor del spin_box1
  dutyCyclePercent2=HMI_get_value("spin_box", "sp2"); // Obtiene el valor del spin_box2

  valor1 = digitalRead(boton1);  //lectura digital del boton 1
  valor2 = digitalRead(boton2);  //lectura digital del boton 2
  
  if (valor1 == HIGH){        //cada que aplasta el boton1 se suma 1 a la bandera1 
    bandera1 += 1;            //bandera 1 = 1 es para prender el led 1
    if (bandera1 == 2){         //si aplasta de nuevo se pone a valor 0 la bandera1
    bandera1 = 0;
    }                         //bandera1 = 0 significa que el led 1 se apaga
  }

  if (valor2 == HIGH){        //cada que aplasta el boton2 se suma 1 a la bandera2 
    bandera2 += 1;            //bandera 2 = 1 es para prender el led 2
    if (bandera2 == 2){         //si aplasta de nuevo se pone a valor 0 la bandera2
    bandera2 = 0;
    }                         //bandera2 = 0 significa que el led 2 se apaga
  }

  // ********************************************************* PARA LED 1 ***************************************************
  
  if (dutyCyclePercent >= 0 && dutyCyclePercent <=100){
    pwmValue = map(dutyCyclePercent, 0, 100, 0, 255);      // Mapea el valor de duty cycle 1 en porcentaje a valores de 0 a 255
    Serial.print("Duty cycle (%) led 1: ");
    Serial.print(" -> PWM value: ");
    Serial.println(pwmValue);
    
    if (bandera1 == 1) {                                  // Solo aqui se prende
      analogWrite(led, pwmValue);
    }else {
      analogWrite(led, 0);                                // Si no cumple se apaga
    }
  }else {
    Serial.println("Ingresa un valor entre 0 y 100.");
  }

  // ********************************************************* PARA LED 2 ***************************************************

  if (dutyCyclePercent2 >= 0 && dutyCyclePercent2 <=100){
    pwmValue2 = map(dutyCyclePercent2, 0, 100, 0, 255);      // Mapea el valor de duty cycle 1 en porcentaje a valores de 0 a 255
    Serial.print("Duty cycle (%) led 2: ");
    Serial.print(" -> PWM value: ");
    Serial.println(pwmValue2);
    
    if (bandera2 == 1) {                                  // Solo aqui se prende
      analogWrite(led2, pwmValue2);
    }else {
      analogWrite(led2, 0);                                // Si no cumple se apaga
    }
  }else {
    Serial.println("Ingresa un valor entre 0 y 100.");
  }
}
