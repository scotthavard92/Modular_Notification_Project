#include <Arduino.h>
#include <ESP8266WiFi.h>


//Enter WiFi Credentials
const char WiFiSSID[] = "My-WiFi-Network"; //Enter WiFi Network Name
const char WiFiPSK[] = "My-WiFi-PassWord"; //Enter Wifi Password

//ifttt information
const char event[] = "This-event-will-be-specified-on-the-IFTTT-Site-such-as-email_me"; //ifttt event
const char privatekey[] = "ifttt-secret-key-is-normally-a-bunch-of-7w6AVPQEUpvmgq3"; //ifttt secret key

//WiFiClient initialization
WiFiClient client;

//String for JSON body
String Value1 = "{\"value1\":\"value1\"}";

void setup() {
Serial.begin(57600);
Serial.println();

WiFiConnect();
iftttConnect();
}

//No Loop needed as of yet
void loop() {

}

//Connect to local WiFi
void WiFiConnect () {
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

//Ping IFTTT
void iftttConnect () {
  
  const int httpPort = 80;
  if (client.connect("maker.ifttt.com", 80)) {
      //Let user know communication with ifttt was succesful
      Serial.println("Connected to ifttt");

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

      //Serial printout for debugging
      Serial.print("POST /trigger/");
      Serial.print(event);
      Serial.print("/with/key/");
      Serial.print(privatekey);
      Serial.println(" HTTP/1.1");
      Serial.println("Host: maker.ifttt.com");
      Serial.println("User-Agent: ESP8266-WiFi/1.0");
      Serial.println("Connection: close");
      Serial.println("Content-Type: application/json");
      Serial.print("Content-Length: ");
      Serial.println(Value1.length());
      Serial.println();
      Serial.println(Value1);
  }
  
  else {
    Serial.println("error connecting to ifttt");
  }
}
 




