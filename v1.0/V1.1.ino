#include <Wire.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>

/***************Definitions***************/
/*****************************************/
//
///

//Accelerometer Definitions
#define MMA8452_ADDRESS 0x1D
#define OUT_X_MSB 0x01 
#define OUT_Z_MSB 0x05 
#define XYZ_DATA_CFG  0x0E 
#define ACCEL_ADDR   0x0D 
#define CTRL_REG1  0x2A 
#define GSCALE 2 

/**********Global Variable Declarations**********/
/************************************************/
//
///

//Hardware Global Variables - One initial value and one active reading 
//for each sensor
float init_acceleration;
u_int init_proximity;
float Acceleration;
u_int Proximity;

//Wireless and IFTTT Global Variables
//Enter WiFi Credentials
const char WiFiSSID[] = "nwname"; //Enter WiFi Network Name
const char WiFiPSK[] = "nwpass"; //Enter Wifi Password

//Enter IFTTT information
const char event[] = "email_me"; //IFTTT event
const char privatekey[] = "secretkey"; //IFTTT secret key

/***************Setup***************/
/***********************************/
//
///

//WiFi Client Initialization
WiFiClient client;

//String for JSON body
String Value1 = "{\"value1\":\"value1\"}";

//Initialize WiFi, IFTTT, both sensors, and store initial values
void setup () {
  Serial.begin(57600);  
  
  WiFiConnect();
  iftttConnect();
  eventTrigger();
  Serial.println("...");

  Wire.begin();
  Accel_Init();
  delay(1000);
  init_acceleration = Init_Accel_Value();
  Serial.print("initial accel value: ");
  Serial.println(init_acceleration);
}

/***************Runtime***************/
/*************************************/
//
///

//Actively calculate sensor values, then compare them to accepted
//range.
void loop () {
  Acceleration=Accel_Calc();  

  Serial.print("Acceleration Value: ");
  Serial.println(Acceleration);
  Serial.println();
  delay(500);

  value_check();

}

/***************Function Definitions***************/
/**************************************************/
//
///

/*****Accelerometer Functions*****/

//Request one byte of information for the accelerometer. 
byte readRegister(byte addressToRead) {
  Wire.beginTransmission(MMA8452_ADDRESS);
  Wire.write(addressToRead);
  Wire.endTransmission(false); 

  Wire.requestFrom(MMA8452_ADDRESS, 1); 

  while(!Wire.available()) ; 
  return Wire.read(); 
}

//Request desired number of bytes from accelerometer. End transmission once 
//number of expected bytes is received.
void readRegisters(byte addressToRead, int bytesToRead, byte * dest) {
  Wire.beginTransmission(MMA8452_ADDRESS);
  Wire.write(addressToRead);
  Wire.endTransmission(false); 

  Wire.requestFrom(MMA8452_ADDRESS, bytesToRead); 

  while(Wire.available() < bytesToRead); 

  for(int x = 0 ; x < bytesToRead ; x++)
    dest[x] = Wire.read();    
}

//Write to accelerometer registers.
void writeRegister(byte addressToWrite, byte dataToWrite) {
  Wire.beginTransmission(MMA8452_ADDRESS);
  Wire.write(addressToWrite);
  Wire.write(dataToWrite);
  Wire.endTransmission(); 
}

//Activate accelerometer. Done by setting active bit.
void MMA8452Active(){
  byte c = readRegister(CTRL_REG1);
  writeRegister(CTRL_REG1, c | 0x01); 
}

//Place accelerometer into standby. Done by clearing active bit.
void MMA8452Standby() {
  byte c = readRegister(CTRL_REG1);
  writeRegister(CTRL_REG1, c & ~(0x01)); 
}

//Read accelerometer data.
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

//Initialize accelerometer.
void Accel_Init() {
  byte c = readRegister(ACCEL_ADDR); 
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
  if(fsr > 8) fsr = 8; //Error check
  fsr >>= 2; // Neat trick, see page 22. 00 = 2G, 01 = 4A, 10 = 8G
  writeRegister(XYZ_DATA_CFG, fsr);

  MMA8452Active();  // Set to active to start reading
}


//Calculate acceleration value in g's.
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

//Calculate initial acceleration value.
float Init_Accel_Value() {
  int c = 0;
  float init_accel;
  while(c < 10) {
    int accelCount[3];  // Stores the 12-bit signed value
    readAccelData(accelCount);  // Read the x/y/z adc values

    // Now we'll calculate the accleration value into actual g's
    float accelG[3];  // Stores the real accel value in g's
    for (int i = 0 ; i < 3 ; i++) {
      accelG[i] = (float) accelCount[i] / ((1<<12)/(2*GSCALE)); 
    }
    if (c=9) {
      init_accel = (accelG[2]);
    }

    delay(300);
    c++;
  }
  
  return init_accel;
}


/*****WiFi Connection*****/
void WiFiConnect() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WiFiSSID, WiFiPSK);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print (".");
  }
  Serial.println();
  Serial.print("Connected,IP address: ");
  Serial.println(WiFi.localIP());
}

/*****IFTTT Functions*****/
//Connect to IFTTT
void iftttConnect() {
  const int httpPort = 80;
  if (client.connect("maker.ifttt.com", 80)) {
    //Let user know communication with ifttt was succesful
    Serial.println("Connected to IFTTT"); 
  }
  else {
    Serial.println("Could not connect to IFTTT");
  }
}

//Event Trigger
void eventTrigger() {
   //Send the POST request
   client.print("POST /trigger/");
   client.print(event);
   client.print("/with/key/");
   client.print(privatekey);
   client.println(" HTTP/1.1");
   client.println("Host: maker.ifttt.com");
   client.println("User-Agent: Arduino/1.0");
   client.println("Connection: close");
   client.println("Content-Type: application/json");
   client.print("Content-Length: ");
   client.println(Value1.length());
   client.println();
   client.println(Value1);
}

/*****Value checking*****/
void value_check() {
  float accel_step = init_acceleration * 0.095;
  if(Acceleration > (accel_step + init_acceleration)){
    Serial.println("Movement Detected - Accel");
    iftttConnect();
    eventTrigger();
  }
  else if ((init_acceleration-accel_step) > Acceleration) {
    Serial.println("Movement Detected - Accel");
    iftttConnect();
    eventTrigger();
  }
}

