#include <SoftwareSerial.h>
#include <Sensirion.h>
#include <QueueList.h>

SoftwareSerial BTSerial(9, 10); // SoftwareSerial(RX, TX)

const byte dataPin =  11;                 // SHTxx serial data
const byte sclkPin =  12;                 // SHTxx serial clock
const unsigned long TRHSTEP   = 5000UL;  // Sensor query period

Sensirion sht = Sensirion(dataPin, sclkPin);

unsigned int rawData;
float temperature;
float humidity;
float dewpoint;

byte measActive = false;
byte measType = TEMP;

unsigned long trhMillis = 0;             // Time interval tracking

QueueList <String> BTQueue;

void setup() 
{
  Serial.begin(9600);
  
  byte stat;
  sht.readSR(&stat);                     // Read sensor status register
  sht.writeSR(LOW_RES);                  // Set sensor to low resolution
  sht.readSR(&stat);                     // Read sensor status register again
  sht.measTemp(&rawData);                // sht.meas(TEMP, &rawData, BLOCK)
  temperature = sht.calcTemp(rawData);
  sht.measHumi(&rawData);                // sht.meas(HUMI, &rawData, BLOCK)
  humidity = sht.calcHumi(rawData, temperature);
  dewpoint = sht.calcDewpoint(humidity, temperature);
  logData();

  
  BTSerial.begin(9600);
}

void loop() 
{
  unsigned long curMillis = millis();          // Get current time

  // Demonstrate non-blocking calls
  if (curMillis - trhMillis >= TRHSTEP) 
  {      // Time for new measurements?
    measActive = true;
    measType = TEMP;
    sht.meas(TEMP, &rawData, NONBLOCK);        // Start temp measurement
    trhMillis = curMillis;
  }
  
  if (measActive && sht.measRdy()) 
  {           // Check measurement status
    if (measType == TEMP) 
    {                    // Process temp or humi?
      measType = HUMI;
      temperature = sht.calcTemp(rawData);     // Convert raw sensor data
      sht.meas(HUMI, &rawData, NONBLOCK);      // Start humi measurement
    } 
    else 
    {
      measActive = false;
      humidity = sht.calcHumi(rawData, temperature); // Convert raw sensor data
      dewpoint = sht.calcDewpoint(humidity, temperature);
      logData();
      sendData();
    }
  }
  
  while (BTSerial.available())
  { // if BT sends something
    byte data = BTSerial.read();
    Serial.write(data); // write it to serial(serial monitor)
  }
  
  while (Serial.available())
  { // if Serial has input(from serial monitor)
    byte data = Serial.read();
    BTSerial.write(data); // write it to BT
  }
}

void logData() 
{
  Serial.print("Temperature = ");   Serial.print(temperature);
  Serial.print(" C, Humidity = ");  Serial.print(humidity);
  Serial.print(" %, Dewpoint = ");  Serial.print(dewpoint);
  Serial.println(" C");
}
void sendData()
{
  int val_int;
  int val_fra;
  char tempBuff[10];
  char humiBuff[10];
  
  String sendDataStr = "";
  
  val_int = (int) temperature;
  val_fra = (int) ((temperature - (float)val_int) * 10);
  snprintf (tempBuff, sizeof(tempBuff), "%d.%d", val_int, val_fra); 
  
  sendDataStr.concat("T:"+String(tempBuff));
    
  val_int = (int) humidity;
  val_fra = (int) ((humidity - (float)val_int) * 10);
  snprintf (humiBuff, sizeof(humiBuff), "%d.%d", val_int, val_fra); 
        
  sendDataStr.concat(", H:"+String(humiBuff));
  
  BTSerial.print(sendDataStr);
  
  Serial.println(sendDataStr);
}

// The following code is only used with error checking enabled
void logError(byte error) 
{
  switch (error) 
  {
  case S_Err_NoACK:
    Serial.println("Error: No response (ACK) received from sensor!");
    break;
  case S_Err_CRC:
    Serial.println("Error: CRC mismatch!");
    break;
  case S_Err_TO:
    Serial.println("Error: Measurement timeout!");
    break;
  default:
    Serial.println("Unknown error received!");
    break;
  }
}

