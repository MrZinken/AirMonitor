/**
 * Dust Sensor with ESP8266 and ThingSpeak upload (SDS011)
 */
#include "SdsDustSensor.h"
#include "SparkFun_SCD4x_Arduino_Library.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include "ThingSpeak.h"

int rxPin = D3;
int txPin = D4;
const int sleeptime = 1000 * 60 * 10;

const char* ssid = "WG_Fit_GmbH_EXT";        // your network SSID (name)
const char* pass = "***********";  // your network password

unsigned long myChannelNumber = 1933391;
const char* myWriteAPIKey = "**********";

WiFiClient client;
SdsDustSensor sds(rxPin, txPin);
SCD4x mySensor;
Adafruit_BME280 bme;

//#define DEBUG    // Herauskommentieren, wenn keine serielle Ausgabe erwünscht wird

#ifdef DEBUG
  #define debug_print(x)  Serial.print(x)
  #define debug_println(x)  Serial.println(x)
  #define debug_begin(x) Serial.begin(x)
#else
  #define debug_print(x)
  #define debug_println(x) 
  #define debug_begin(x)
#endif

void setup() {

  debug_begin(9600);
 

  sds.begin();
  Wire.begin(D6, D5);
  bme.begin(0x76);
  mySensor.begin(0x62); //   mySensor.begin(0x62);


  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);  // Initialize ThingSpeak

  // Connect or reconnect to WiFi
  if (WiFi.status() != WL_CONNECTED) {
    debug_print("Attempting to connect to SSID: ");
    debug_println(ssid);

    int attempts = 0;
    while (attempts != 5) {
      WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      // Serial.print(".");
      delay(10000);
      attempts++;
      // Serial.println(attempts);
      if (WiFi.status() == WL_CONNECTED) {
        debug_println("\nConnected.");
        break;
      } else {
        debug_println("Could not connect to Wifi. Attempts: " + String(attempts) + "/5");
      }
    }
  }
}


void loop() {

  //WakeUp Sds Sensor
  sds.wakeup();  
  delay(10000); // waiting 15 seconds before measuring
  PmResult pm = sds.queryPm();

  //Messure and fill message
  ThingSpeak.setField(1, pm.pm25);
  ThingSpeak.setField(2, pm.pm10);
  delay(500);
  ThingSpeak.setField(3, bme.readHumidity());
  ThingSpeak.setField(4, bme.readTemperature());
  ThingSpeak.setField(5, bme.readPressure() / 100.0F);
  delay(500);
  ThingSpeak.setField(6, mySensor.getCO2());
  //Send message
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if (x == 200) {
    debug_println("Channel update successful.");
  } else {
    debug_println("Problem updating channel. HTTP error code " + String(x));
  }

  //Sds Sensor sleep
  WorkingStateResult state = sds.sleep();
  if (state.isWorking()) {
    debug_println("Problem with sleeping the sensor.");
  } else {
    debug_println("\nSensor is sleeping");
  }
  //wait before next messurement
  delay(sleeptime);
}
