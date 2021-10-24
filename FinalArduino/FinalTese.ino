#include <GSM.h>
#include "Seeed_BME280.h"
#include <Wire.h>

#define PINNUMBER "" //Define o pin do cartão SIM
 
BME280 bme280;

// initialize the library instance
GSM gsmAccess; // include a 'true' parameter for debug enabled
GSM_SMS sms;
// char array of the telephone number to send SMS
char remoteNumber[9]= ""; //Número de telemóvel para qual será mandada a informação

volatile unsigned long FastNeutron = 0; // Variável que irá servir para fazer a taxa de neutrões rápidos
const int pinfast = 19; // Pin digital de entrada dos sinais dos neutrões rápidos
volatile unsigned long ThermalNeutron = 0; // Variável que irá servir para fazer a taxa de neutrões térmicos
const int pinthermal = 18; // Pin digital de entrada dos sinais dos neutrões térmicos

long fast = 0;
long thermal = 0;

float Pressure;
float Temperature;
float Humidity;

String p = "Pressure:";
String up = "hPa";
String t = "Temperature:";
String h = "Humidity:";
String uh = "%";
String fn = "Fast Neutrons:";
String ufn = "Count/h";
String tn = "Thermal Neutrons:";
String utn = "Count/h";
String bat = "Voltage:";
String b = "V"; 

unsigned long antes = 0;
unsigned long interval = 1800000; //Intervalo de amostragem

static const int PowerPin = 22;

void pulsefast(){ //Função que recebe e conta os pulsos dos neutrões rápidos
  FastNeutron++;}

void pulsethermal(){ //Função que recebe e conta os pulsos dos neutrões térmicos
  ThermalNeutron++;}

void setup(){

  // initialize serial communications
  Serial.begin(9600);
  
  pinMode(PowerPin, OUTPUT);
  //to turn on the GSM module -software switch instead Power Key
  digitalWrite(PowerPin, LOW);
  delay (2000);
  digitalWrite (PowerPin, HIGH);

  // connection state
  boolean notConnected = true;
  
  // Start GSM shield
  // If your SIM has PIN, pass it as a parameter of begin() in quotes
  while(notConnected){
    if(gsmAccess.begin(PINNUMBER) == GSM_READY){
      notConnected = false;
    }
    else{
      delay(1000);}    
  }
  
  if(!bme280.init()){
    Serial.println("Device error!");
    delay(1000);
  }
    
  pinMode(pinfast, INPUT); //Pins de entrada
  pinMode(pinthermal, INPUT);
  attachInterrupt(digitalPinToInterrupt(pinfast), pulsefast, RISING);
  attachInterrupt(digitalPinToInterrupt(pinthermal), pulsethermal, RISING); //Rising basicamente dá 1, quando o pulso vai do 0 ao 1

}

void loop(){

  unsigned long agora = millis();
   
  if(agora - antes >= interval){
    antes = agora;
    
    int battery = analogRead(A2);
    float voltage = battery * (12.6 / 1023.0); //Valor que guarda a tensão das baterias
    float batpec = voltage * 27.778 - 250; //Percentagem das baterias

    Pressure = bme280.getPressure();
    Temperature = bme280.getTemperature();
    Humidity = bme280.getHumidity();
    
    // send the message
    sms.beginSMS(remoteNumber);
    sms.print(";" + t + ";");
    sms.print(Temperature,2);
    sms.print(";");
    sms.print("Degrees Celsius");
    sms.print(";" + p + ";");
    sms.print(Pressure,2);
    sms.print(";");
    sms.print(up + ";" + h + ";");
    sms.print(Humidity,3);
    sms.print(";" + uh + ";" + fn + ";" + 2*FastNeutron + ";" + ufn + ";" + tn + ";" + 2*ThermalNeutron + ";" + utn + ";" + bat + ";");
    sms.print(voltage,2);
    sms.print(";" + b + ";");
    sms.print(batpec,2);
    sms.print(";" + uh + ";");
    sms.endSMS();
    
    FastNeutron = 0;
    ThermalNeutron =0;
  }
}
