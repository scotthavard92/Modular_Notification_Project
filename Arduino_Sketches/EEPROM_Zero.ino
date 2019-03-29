#include <EEPROM.h>

void setup() {
  Serial.begin(9600);
  EEPROM.begin(512);
  String existingString = read_String(10);
  int stringLength = existingString.length();
  writeZero(10, stringLength);
  Serial.println("Zeroing Complete");
}

void loop() {
  String value = read_String(10);
  Serial.print("Value: ");
  Serial.println(value);
  delay(1000);
}

String read_String(char add)
{
  int i;
  char data[100]; //Max 100 Bytes
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
}
 
