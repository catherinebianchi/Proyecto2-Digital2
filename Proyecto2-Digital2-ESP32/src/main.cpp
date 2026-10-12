//******************************************/
// Universidad del Valle de Guatemala
// BE3029 - Electronica Digital 2
// Catherine Bianchi y Camila Tello
// Proyecto 2: Protocolos de Comunicación Serial y Timers
// MCU: ESP32 dev kit 1.0
//******************************************/
//******************************************/
// Librerias
//******************************************/
#include <Arduino.h>
#include <stdint.h>
#include <ESP32SPISlave.h>
#include <LiquidCrystal.h>
#include <Wire.h>

//******************************************/
// Definiciones
//******************************************/
//Pines LEDs + Slave
#define ledVerde 4
#define ledAmarillo 16
#define ledRojo 17
#define QUEUE_SIZE 1

//Pines pantalla
#define pot1 34
#define rs 13
#define en 14
#define d4 26
#define d5 25
#define d6 27
#define d7 33

//I2C
#define sdaI2C 22
#define sclI2C 21

//******************************************/
// Prototipos de funciones
//******************************************/
void leerPotenciometro(void);
void mostrarLCD(void);
void recibirI2C(int cantidad);
void enviarI2C(void);

//******************************************/
// Variables globales
//******************************************/
//Slave y operación de LEDs
uint8_t txBuff[4] = {0};
uint8_t rxBuff[4] = {0};
uint8_t ledActivo = 0;
unsigned long tFin = 0;
const uint8_t pinLed[4] = {0, ledVerde, ledAmarillo, ledRojo};
ESP32SPISlave slave;

//Pantalla 
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
int lecturaADC = 0; //Valor original: 0 a 4095
int valor8bits = 0; //Valor para la pantalla: 0 a 255
float voltios = 0;
const char iniciaLED[4] = {'-','G','Y','R'};
char ultimoLED = '-';

//I2C
const uint8_t direccionI2C = 0x28; //Dirección de del ESP32 en el bus I2C
volatile bool medicionPendiente = false; //Conexión al nucleo
volatile uint16_t lecturaNucleo = 0;
unsigned long ultimaActualizacion = 0;


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

  slave.setDataMode(SPI_MODE0); //Regla del ritmo para leer bits - polaridad baja y primer flanco
  slave.setQueueSize(QUEUE_SIZE);
  slave.begin(VSPI); //CS:5, CLK:18, MOSI:23, MISO:19
  Serial.println("Esclavo SPI listo");

  pinMode(pot1, INPUT);
  analogReadResolution(12);
  lcd.begin(16,2);

  //Imagen de las tres columnas de la LCD
  lcd.setCursor(0,0);
  lcd.print("Pot1:");
  lcd.setCursor(6,0);
  lcd.print("Pot1:");  
  lcd.setCursor(12,0);
  lcd.print("LED:");  

  //Inicia la comunicación I2C
  Wire.onReceive(recibirI2C);
  Wire.onRequest(enviarI2C);

  if(!Wire.begin(direccionI2C, sdaI2C, sclI2C, 100000)){ //Enciende I2C como esclavo con 0x28
    Serial.println("Error al iniciar I2C"); //If, no salió bien = imprimir error
  }
  else{
    Serial.println("ESP32 I2C listo");
  }

}
//******************************************/
// Loop Principal
//******************************************/
void loop() {
  //¿No hay nada pendiente?
  if(slave.hasTransactionsCompletedAndAllResultsHandled()){
    slave.queue(txBuff, rxBuff, 4);
    slave.trigger();
  }
  //¿Ya terminó de enviar un mensaje completo?
  if(slave.hasTransactionsCompletedAndAllResultsReady(QUEUE_SIZE)){
    slave.numBytesReceivedAll();

    uint8_t led = rxBuff[0]; //LED
    uint16_t ms = (rxBuff[1] << 8) | rxBuff[2]; //TIEMPO, viaja en 2 bytes
    Serial.printf("Recibido: %d %d %d %d\n", rxBuff[0], rxBuff[1], rxBuff[2], rxBuff[3]);

    if (led >= 1 && led <= 3) {
      ultimoLED = iniciaLED[led];
      Serial.printf("Led %d encendido por %d ms\n", led, ms);
      digitalWrite(pinLed[led], HIGH);
      ledActivo = led;
      tFin = millis() + ms;
    }
  }
  //Si hay un LED encendido y ya pasó el tiempo = APAGAR
  //No se utiliza delay() para no congelar programa
  if (ledActivo && millis() >= tFin) {
    digitalWrite(pinLed[ledActivo], LOW);
    ledActivo = 0;
  }

  //Parte 3: Conexión con el I2C
    //  'M' enviada por la Nucleo.
  if (medicionPendiente)
  {
    medicionPendiente = false;

    uint16_t muestra = analogRead(pot1);
    lecturaNucleo = muestra;

    
#if defined(CONFIG_IDF_TARGET_ESP32) //Evalúa al compilar
    uint8_t respuesta[2] = {
      (uint8_t)(muestra >> 8), //Parte alta (8 bits de arriba)
      (uint8_t)(muestra & 0xFF) //Parte baja (8 bits de abajo)
    };

    Wire.slaveWrite(respuesta, 2);
#endif
  }

  // LCD a 300ms - para leer potenciómetro y actualizar
  if (millis() - ultimaActualizacion >= 300)
  {
    ultimaActualizacion = millis();

    leerPotenciometro();
    mostrarLCD();
  }
  }


//******************************************/
// Otras funciones
//******************************************/

void leerPotenciometro(void){
  // Se hace la lectura 
  lecturaADC = analogRead(pot1);

  // se muestra la imagen de 8 bits (0 a 255)
  valor8bits = lecturaADC / 16;

  // Leer el voltaje del mismo pin y convertir mV a V
  voltios = analogReadMilliVolts(pot1) / 1000.0;
}

void mostrarLCD(void){
  //Primera columna: voltaje
  lcd.setCursor(0, 1);
  lcd.print("     ");
  lcd.setCursor(0, 1);
  lcd.print(voltios, 2);
  lcd.print('V');

  // Segunda columna: lectura entre 0 y 255
  lcd.setCursor(6, 1);
  lcd.print("   ");
  lcd.setCursor(6, 1);
  lcd.print(valor8bits);

  // Tercera columna: inicial del ultimo LED activado
  lcd.setCursor(13, 1);
  lcd.print(ultimoLED);  
}

//Cuando la Nucleo envia 'M', se marca una medicion pendiente.
void recibirI2C(int cantidad){
  while(Wire.available()){
    if(Wire.read() =='M'){
      medicionPendiente = true;
    }
  }
}

//Para enviar información al núcleo
void enviarI2C(void){
  #if !defined(CONFIG_IDF_TARGET_ESP32)
  uint16_t muestra = lecturaNucleo;

  Wire.write((uint8_t)(muestra>>8));
  Wire.write((uint8_t)(muestra & 0xFF));
  #endif
}


