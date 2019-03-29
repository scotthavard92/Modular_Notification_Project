
const byte numChars = 32;
char receivedChars[numChars]; // an array to store the received data
char tempChars[numChars]; 

char messageFromPC1[numChars] = {0};
char messageFromPC2[numChars] = {0};
char messageFromPC3[numChars] = {0};
char messageFromPC4[numChars] = {0};

boolean newData = false;
boolean valueExists = false;

void setup() {
 Serial.begin(9600);
 Serial.println("<Arduino is ready>");
}

void loop() {
 recvWithEndMarker();
 strcpy(tempChars, receivedChars);
 parseData();
 printString();
 delay(1000);
 
}

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
  }
  valueExists = true;
 }
}


void parseData() {
    if (valueExists == true) {
        // split the data into its parts
        char * strtokIndx; // this is used by strtok() as an index
  
        strtokIndx = strtok(tempChars,",");      // get the first part - the string
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

void printString() {
  Serial.println(receivedChars);
  //Serial.println(tempChars);
  Serial.println(messageFromPC1);
  Serial.println(messageFromPC2);
  Serial.println(messageFromPC3);
  Serial.println(messageFromPC4);
  Serial.println(valueExists);
}

