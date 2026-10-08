//******************************************/
// Universidad del Valle de Guatemala
// BE3029 - Electronica Digital 2
// Pablo Mazariegos
// 21/07/2025
// Proyecto 2 
// MCU: ESP32 dev kit 1.0
//******************************************/
//******************************************/
// Librerias
//******************************************/
#include <Arduino.h>
#include <stdint.h>
#include <ESP32SPISlave.h>

//******************************************/
// Definiciones
//******************************************/
#define ledVerde 33
#define ledAmarillo 32
#define ledRojo 25
#define QUEUE_SIZE 1

//******************************************/
// Prototipos de funciones
//******************************************/

//******************************************/
// Variables globales
//******************************************/
uint8_t txBuff[4] = {0};
uint8_t rxBuff[4] = {0};
uint8_t ledActivo = 0;
unsigned long tFin = 0;
const uint8_t pinLed[4] = {0, ledVerde, ledAmarillo, ledRojo};


ESP32SPISlave slave;

//******************************************/
// Configuración
//******************************************/
void setup() {
  Serial.begin(115200);

  pinMode(ledAmarillo, OUTPUT);
  pinMode(ledVerde, OUTPUT);
  pinMode(ledRojo, OUTPUT);

  digitalWrite(ledAmarillo, LOW);
  digitalWrite(ledVerde, LOW);
  digitalWrite(ledRojo, LOW);

  slave.setDataMode(SPI_MODE0);
  slave.setQueueSize(QUEUE_SIZE);
  slave.begin(VSPI); //CS:5, CLK:18, MOSI:23, MISO:19
  Serial.println("Esclavo SPI listo");

}
//******************************************/
// Loop Principal
//******************************************/
void loop() {
  if(slave.hasTransactionsCompletedAndAllResultsHandled()){
    slave.queue(txBuff, rxBuff, 4);
    slave.trigger();
  }
  if(slave.hasTransactionsCompletedAndAllResultsReady(QUEUE_SIZE)){
    slave.numBytesReceivedAll();

    uint8_t led = rxBuff[0];
    uint16_t ms = (rxBuff[1] << 8) | rxBuff[2];
    Serial.printf("Recibido: %d %d %d %d\n", rxBuff[0], rxBuff[1], rxBuff[2], rxBuff[3]);

    if (led >= 1 && led <= 3) {
      Serial.printf("Led %d encendido por %d ms\n", led, ms);
      digitalWrite(pinLed[led], HIGH);
      ledActivo = led;
      tFin = millis() + ms;
    }
  }

  if (ledActivo && millis() >= tFin) {
    digitalWrite(pinLed[ledActivo], LOW);
    ledActivo = 0;
  }

  }


//******************************************/
// Otras funciones
//******************************************/
