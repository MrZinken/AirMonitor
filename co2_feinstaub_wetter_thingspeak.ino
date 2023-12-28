


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
const int sleeptime = 1000 * 60 * 30;

const char* ssid = "WG_Fit_GmbH_EXT";        // your network SSID (name)
const char* pass = "Liebdochwenduwillst18";  // your network password

unsigned long myChannelNumber = 1933391;
const char* myWriteAPIKey = "X9L20E60JG38677Y";

WiFiClient client;
SdsDustSensor sds(rxPin, txPin);
SCD4x mySensor;
Adafruit_BME280 bme;




void setup() {

  Serial.begin(9600);
  Serial.setTimeout(2000);

  Wire.begin(D6, D5);
  bme.begin(0x76);
  mySensor.begin(0x62); //   mySensor.begin(0x62);

  // Wait for serial to initialize.
  while (!Serial) {}


  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);  // Initialize ThingSpeak

  // Connect or reconnect to WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);

    int attempts = 0;
    while (attempts != 5) {
      WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      // Serial.print(".");
      delay(10000);
      attempts++;
      // Serial.println(attempts);
      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConnected.");
        break;
      } else {
        Serial.println("Could not connect to Wifi. Attempts: " + String(attempts) + "/5");
      }
    }
  }
}

void loop() {


  
  sds.begin();
  sds.wakeup();

  delay(10000); // waiting 15 seconds before measuring
  PmResult pm = sds.queryPm();



  ThingSpeak.setField(1, pm.pm25);
  ThingSpeak.setField(2, pm.pm10);
  delay(500);
  ThingSpeak.setField(3, bme.readHumidity());
  ThingSpeak.setField(4, bme.readTemperature());
  ThingSpeak.setField(5, bme.readPressure() / 100.0F);
  delay(500);
  ThingSpeak.setField(6, mySensor.getCO2());

  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if (x == 200) {
    Serial.println("Channel update successful.");
  } else {
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }



  WorkingStateResult state = sds.sleep();
  if (state.isWorking()) {
    Serial.println("Problem with sleeping the sensor.");
  } else {
    Serial.println("\nSensor is sleeping");
  }
  delay(sleeptime);
}