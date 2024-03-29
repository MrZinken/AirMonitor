/**
 * Dust Sensor with ESP8266 and ThingSpeak upload (SDS011)
 */
 #include "SdsDustSensor.h"
 #include "ThingSpeak.h"
 #include <ESP8266WiFi.h>

 int rxPin = D3;
 int txPin = D4;
 const int sleeptime = 1000*60*30;

 SdsDustSensor sds(rxPin, txPin);

 const char* ssid = "WG_Fit_GmbH_EXT";   // your network SSID (name)
 const char* pass = "***********";   // your network password

 WiFiClient  client;

 unsigned long myChannelNumber = 942047;
 const char * myWriteAPIKey = "********";

 void setup() {

   Serial.begin(9600);
   Serial.setTimeout(2000);

   // Wait for serial to initialize.
   while(!Serial) { }


   WiFi.mode(WIFI_STA);
   ThingSpeak.begin(client);  // Initialize ThingSpeak

       // Connect or reconnect to WiFi
   if(WiFi.status() != WL_CONNECTED){
     Serial.print("Attempting to connect to SSID: ");
     Serial.println(ssid);

     int attempts = 0;
     while(attempts != 5){
           WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network. Change this line if using open or WEP network
           // Serial.print(".");
           delay(10000);
           attempts++;
           // Serial.println(attempts);
           if (WiFi.status() == WL_CONNECTED) {
             Serial.println("\nConnected.");
             break;
           }
           else {
           Serial.println("Could not connect to Wifi. Attempts: " + String(attempts) + "/5");
           }
     }
   }

  
 }

 void loop() {

   // Serial.println("I'm awake.");

   sds.begin();

   Serial.println(sds.queryFirmwareVersion().toString()); // prints firmware version
   Serial.println(sds.setQueryReportingMode().toString()); // ensures sensor is in 'query' reporting mode


   sds.wakeup();
   // digitalWrite(LED_BUILTIN, LOW);   Turn the LED on (Note that LOW is the voltage level
   delay(15000); // waiting 15 seconds before measuring

   PmResult pm = sds.queryPm();
   if (pm.isOk()) {
     Serial.print("PM2.5 = ");
     Serial.print(pm.pm25);
     Serial.print(", PM10 = ");
     Serial.println(pm.pm10);

     // if you want to just print the measured values, you can use toString() method as well
     Serial.println(pm.toString());

     ThingSpeak.setField(1, pm.pm25);
     ThingSpeak.setField(2, pm.pm10);

     int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
     if(x == 200){
       Serial.println("Channel update successful.");
     }
     else{
       Serial.println("Problem updating channel. HTTP error code " + String(x));
     }



   } else {
     Serial.print("Could not read values from sensor, reason: ");
     Serial.println(pm.statusToString());
   }

   

   WorkingStateResult state = sds.sleep();
   if (state.isWorking()) {
     Serial.println("Problem with sleeping the sensor.");
   } else {
     Serial.println("\nSensor is sleeping");

   }
   delay(sleeptime);
 }
