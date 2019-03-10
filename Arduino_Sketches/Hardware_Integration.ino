#include <Wire.h>

/*****Definitions*****/
//
///

//Proximity Definitions
#define COM_REG 0x80
#define PROX_ADDR 0x13
#define PROX_DATA_RATE 0x82
#define PROX_DATA_RESULT 0x87

//Accelerometer Definitions
#define MMA8452_ADDRESS 0x1D
#define OUT_X_MSB 0x01 
#define OUT_Z_MSB 0x05 
#define XYZ_DATA_CFG  0x0E 
#define ACCEL_ADDR   0x0D 
#define CTRL_REG1  0x2A 
#define GSCALE 2 

/*****Global Variable Declarations*****/
//
///

//Proximity Declarations
unsigned int prox_data;

//Accelerometer Declarations

/*****Setup*****/
//
///

void setup () {
  Serial.begin(57600); //Baud rate considerations 
  Wire.begin();  
  Accel_Init();
  Prox_Init();
}

/*****Runtime*****/
//
///

//Main Loop
void loop () {
  u_int Proximity=Prox_Calc();
  float Acceleration=Accel_Calc();  
  
  Serial.print("Proximity Value: ");
  Serial.println(Proximity);
  Serial.println();
  delay(500);

  Serial.print("Acceleration Value: ");
  Serial.println(Acceleration);
  Serial.println();
  delay(500);

}

/*****Function Definitions*****/
byte readRegister(byte addressToRead) {
  Wire.beginTransmission(MMA8452_ADDRESS);
  Wire.write(addressToRead);
  Wire.endTransmission(false); //endTransmission but keep the connection active

  Wire.requestFrom(MMA8452_ADDRESS, 1); //Ask for 1 byte, once done, bus is released by default

  while(!Wire.available()) ; //Wait for the data to come back
  return Wire.read(); //Return this one byte
}

void readRegisters(byte addressToRead, int bytesToRead, byte * dest) {
  Wire.beginTransmission(MMA8452_ADDRESS);
  Wire.write(addressToRead);
  Wire.endTransmission(false); //endTransmission but keep the connection active

  Wire.requestFrom(MMA8452_ADDRESS, bytesToRead); //Ask for bytes, once done, bus is released by default

  while(Wire.available() < bytesToRead); //Hang out until we get the # of bytes we expect

  for(int x = 0 ; x < bytesToRead ; x++)
    dest[x] = Wire.read();    
}

void writeRegister(byte addressToWrite, byte dataToWrite) {
  Wire.beginTransmission(MMA8452_ADDRESS);
  Wire.write(addressToWrite);
  Wire.write(dataToWrite);
  Wire.endTransmission(); //Stop transmitting
}

void MMA8452Active(){
  byte c = readRegister(CTRL_REG1);
  writeRegister(CTRL_REG1, c | 0x01); //Set the active bit to begin detection
}

void MMA8452Standby() {
  byte c = readRegister(CTRL_REG1);
  writeRegister(CTRL_REG1, c & ~(0x01)); //Clear the active bit to go into standby
}

void readAccelData(int *destination) {
  byte rawData[6];  // x/y/z accel register data stored here

  readRegisters(OUT_X_MSB, 6, rawData);  // Read the six raw data registers into data array

  // Loop to calculate 12-bit ADC and g value for each axis
  for(int i = 0; i < 3 ; i++)
  {
    int gCount = (rawData[i*2] << 8) | rawData[(i*2)+1];  //Combine the two 8 bit registers into one 12-bit number
    gCount >>= 4; //The registers are left align, here we right align the 12-bit integer

    // If the number is negative, we have to make it so manually (no 12-bit data type)
    if (rawData[i*2] > 0x7F)
    {  
      gCount -= 0x1000;
    }

    destination[i] = gCount; //Record this gCount into the 3 int array
  }
}

void Accel_Init() {
  byte c = readRegister(ACCEL_ADDR);  // Read WHO_AM_I register
  if (c == 0x2A) {  
    Serial.println("MMA8452Q is online...");
  }
  else {
    Serial.print("Could not connect to MMA8452Q: 0x");
    Serial.println(c, HEX);
    while(1) ; // Loop forever if communication doesn't happen
  }
  MMA8452Standby();  // Must be in standby to change registers

  // Set up the full scale range to 2, 4, or 8g.
  byte fsr = GSCALE;
  if(fsr > 8) fsr = 8; //Easy error check
  fsr >>= 2; // Neat trick, see page 22. 00 = 2G, 01 = 4A, 10 = 8G
  writeRegister(XYZ_DATA_CFG, fsr);

  //The default data rate is 800Hz and we don't modify it in this example code

  MMA8452Active();  // Set to active to start reading
}

void Prox_Init() {
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

u_int Prox_Calc() {
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
  }

  //float Prox_Value = (Prox_Result[0] << 8)|Prox_Result[1];
  float Prox_Value2 = ((Prox_Result[0] * 256) + Prox_Result[1]);

  return Prox_Value2;
}

float Accel_Calc() {
  int accelCount[3];  // Stores the 12-bit signed value
  readAccelData(accelCount);  // Read the x/y/z adc values

  // Now we'll calculate the accleration value into actual g's
  float accelG[3];  // Stores the real accel value in g's
  for (int i = 0 ; i < 3 ; i++) {
    accelG[i] = (float) accelCount[i] / ((1<<12)/(2*GSCALE));  
  }

  float Accel=(accelG[2]);
  return Accel;
}

