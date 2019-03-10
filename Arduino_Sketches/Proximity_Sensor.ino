#include <Wire.h>

//Proximity Sensor Address 0x13, 0x26(write), 0x27 (read)
#define COM_REG 0x80
#define PROX_ADDR 0x13
#define PROX_DATA_RATE 0x82
#define PROX_DATA_RESULT 0x87 

unsigned int prox_data;

void setup() {
 Serial.begin(9600); //Baud rate considerations 
 Wire.begin();
 
 Wire.beginTransmission(PROX_ADDR);
 Wire.write(0x80); //Send a bit asking for data register - Command Register: 80
 Wire.write(0xFF); //Enable measurements for both Proximity and for Ambient Light
 Wire.endTransmission(); //Stop sending info from MCU to Sensor

 Wire.beginTransmission(PROX_ADDR);
 Wire.write(0x82); //
 Wire.write(0x00); // Set rate at 1.95 Measurements/Sec
 Wire.endTransmission();

}

void loop() {
  unsigned int Prox_Result[2];
  int i=0;
  while (i<2) {
    Wire.beginTransmission(PROX_ADDR);
    Wire.write(135+i);
    Wire.endTransmission();
    Wire.requestFrom(PROX_ADDR, 1);
    if(Wire.available() == 1) {
      Prox_Result[i] = Wire.read();
    }
    i++;
    delay(300);
  }
  
  //float Prox_Value = (Prox_Result[0] << 8)|Prox_Result[1];
  float Prox_Value2 = ((Prox_Result[0] * 256) + Prox_Result[1]);
  
  //Serial.print("ProxV1: ");
  //Serial.println(Prox_Value);
  Serial.print("ProxV2: ");
  Serial.println(Prox_Value2);
  
  
  delay(500);
}
