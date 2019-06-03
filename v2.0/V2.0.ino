#include <Wire.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>

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
float init_accel0;
float init_accel1;
float init_accel2;
float accelG[3];

//Serial Communication Variables
const byte numChars = 128;
char receivedChars[numChars]; // an array to store the received data
char tempChars[numChars]; 

//Wifi SSID
char messageFromPC1[numChars] = {0};
// WiFi PSK
char messageFromPC2[numChars] = {0};
//IFTTT Event Name
char messageFromPC3[numChars] = {0};
//IFTTT Key
char messageFromPC4[numChars] = {0};

//Value checks
boolean newData = false;
boolean valueExists = false;

//EEPROM Address
char addr = 10;
unsigned char eepromOccupied;

//Button Declaration
int buttonState = 0;
const int buttonPin = 4;

//IFTTT testing
const char event[] = "email_me"; //IFTTT event
const char privatekey[] = ".."; //IFTTT secret key

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
  Serial.begin(9600); 
  EEPROM.begin(512);
  pinMode(5, OUTPUT);
  pinMode(buttonPin, INPUT);
  eepromOccupied = EEPROM.read(addr); 

  //If there is no stored on the device, move to loop().
  if (eepromOccupied == 0) {
    Serial.println("No Credentials Stored on Setup");
    delay(2000);
    digitalWrite(5, HIGH);
  }

  //If there is information stored on the EEPROM, pull that information and connect to WIFI.
  else {
    Serial.println("Credentials Stored");
    digitalWrite(5, LOW);
    valueExists = true;
    String storedValue = read_String(addr);
    storedValue.toCharArray(receivedChars, numChars);
    strcpy(tempChars, receivedChars);
    parseData(tempChars);
    Serial.println(messageFromPC1);

    WiFiConnect();
    iftttConnect();
    eventTrigger();
    Serial.println("...");

    Wire.begin();
    accelInit();
    delay(1000);
    init_acceleration = initAccelValue();
    Serial.print("initial accel value: ");
    Serial.println(init_acceleration, 4);
  }
}

/***************Runtime***************/
/*************************************/
//
///

void loop () {
  //If there is no information, then keep looking for a Serial connection.
  if (eepromOccupied == 0) {
    digitalWrite(5, HIGH);
    Serial.println("No Credentials Stored on Loop");
    delay(500);
      if (Serial.available() > 0) {
        Serial.println("Serial Available on Loop");
        digitalWrite(5, LOW);
        while(Serial.available() > 0) {
          Serial.println("Connection Loop Entered");
          recvWithEndMarker();
          delay(1000);
        }
        
        writeString(addr, receivedChars);
        strcpy(tempChars, receivedChars);
        parseData(tempChars);
        
        WiFiConnect();
        iftttConnect();
        eepromOccupied = 1;
        delay(500);

        Wire.begin();
        accelInit();
        delay(1000);
        init_acceleration = initAccelValue();
        Serial.print("initial accel value: ");
        Serial.println(init_acceleration, 4);
      }
  }
  
  else {
    //Credentials are stored, don't check for serial but go through accel calculations.
    digitalWrite(5, LOW);
    Serial.println("Credentials Stored in Loop");

    Acceleration=accelCalc();  

    Serial.print("Acceleration Value: ");
    Serial.println(Acceleration, 4);
    Serial.println();
    delay(500);
  
    valueCheck();
    Serial.println(messageFromPC1);
    String checkString = read_String(addr);
    Serial.println(checkString);
  }

  //Clicking the button clears the EEPROM.
  buttonState = digitalRead(buttonPin);
  if (buttonState == HIGH) {
    String existingString = read_String(addr);
    int stringLength = existingString.length();
    writeZero(addr,stringLength);
    eepromOccupied = 0;
    valueExists = false;
    Serial.println("EEPROM cleared");
    delay(500);
    digitalWrite(5, HIGH);
  }
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
void accelInit() {
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
float accelCalc() {
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
float initAccelValue() {
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
  WiFi.begin(messageFromPC1, messageFromPC2);
  Serial.print("Connecting");
  int i = 0;
  while (i<8) {
    i=i+1;
    delay(1000);
    Serial.print (".");
    Serial.print (i);
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println();
    Serial.print("Could not connect to WiFi");
  }
  else if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.print("Connected,IP address: ");
    Serial.println(WiFi.localIP());
  } 
}  

/*****IFTTT Functions*****/
//Connect to IFTTT
void iftttConnect() {
  const int httpPort = 80;
  if (client.connect("maker.ifttt.com", 80)) {
    //Let user know communication with ifttt was succesful
    Serial.println("Connected to IFTTT"); 
    Serial.println(messageFromPC3);
    Serial.println(messageFromPC4);
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

/*****Value Checking*****/
void valueCheck() {
  float accel_step = init_acceleration * 0.1;
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

/*****Serial Communication*****/

//Read characters until end marker.
void recvWithEndMarker() {
 static byte ndx = 0;
 char endMarker = '\n';
 char rc;
 
 // if (Serial.available() > 0) {
 while (Serial.available() > 0 && newData == false) {
  rc = Serial.read();

  if (rc != endMarker) {
    receivedChars[ndx] = rc;
    ndx++;
    if (ndx >= numChars) {
      ndx = numChars - 1;
    }
  }
  else {
    receivedChars[ndx] = '\0'; // terminate the string
    ndx = 0;
    newData = true;
    valueExists = true;
  }
 }
}

//Parse the inputted string..
void parseData(char* infoParse) {
  if (valueExists == true) {
    // split the data into its parts
    char * strtokIndx; // this is used by strtok() as an index

    strtokIndx = strtok(infoParse,",");      // get the first part - the string
    strcpy(messageFromPC1, strtokIndx); // copy it to messageFromPC

    strtokIndx = strtok(NULL, ","); 
    strcpy(messageFromPC2, strtokIndx);

    strtokIndx = strtok(NULL, ","); 
    strcpy(messageFromPC3, strtokIndx);

    strtokIndx = strtok(NULL, ","); 
    strcpy(messageFromPC4, strtokIndx);
    
    newData = false;
  }
}

//Print string over Serial. Helpful for debugging.
void printString() {
  Serial.println(receivedChars);
  //Serial.println(tempChars);
  Serial.println(messageFromPC1);
  Serial.println(messageFromPC2);
  Serial.println(messageFromPC3);
  Serial.println(messageFromPC4);
  Serial.println(valueExists);
}

/*****EEPROM Write*****/
//Write to the devices memory.
void writeString(char add,String data)
{
  int _size = data.length();
  int i;
  for(i=0;i<_size;i++)
  {
    EEPROM.write(add+i,data[i]);
  }
  EEPROM.write(add+_size,'\0');   //Add termination null character for String Data
  EEPROM.commit();
}
 
//Read string from EEPROM
String read_String(char add)
{
  int i;
  char data[128]; //Max 100 Bytes
  int len=0;
  unsigned char k;
  k=EEPROM.read(add);
  while(k != '\0' && len<500)   //Read until null character
  {    
    k=EEPROM.read(add+len);
    data[len]=k;
    len++;
  }
  data[len]='\0';
  return String(data);
}

//Clear the EEPROM.
void writeZero(char add, int stringLength)
{
  int _size = stringLength;
  int i;
  char zero = 0;
  for(i=0;i<_size;i++)
  {
    EEPROM.write(add+i,zero);
  }
  EEPROM.write(add+_size,'\0');   //Add termination null character for Zero
  EEPROM.commit();

  int x=0;
  while (x<_size) {
    receivedChars[x] = 0;
    x = x+1;
  }
  receivedChars[_size+1] = '/0';

  int y=0;
  while (x<_size) {
    tempChars[y] = 0;
    y = y+1;
  }
  tempChars[_size+1] = '/0';
}
