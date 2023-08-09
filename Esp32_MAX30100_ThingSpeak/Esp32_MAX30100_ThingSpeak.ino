
#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
 
// Set our wifi name and password
const char* ssid = "******";// write your ssid
const char* password = "********";//write your ssid password here
 
// Your thingspeak channel url with api_key query
String serverName = "https://api.thingspeak.com/update?api_key=********";//add your Thingspeak key here
 
// Assign some variables to allow us read and send data every minute
unsigned long lastTime = 0;
unsigned long timerDelay = 5000;
 
 // time period interval in which sensor collects readings
#define REPORTING_PERIOD_MS     500

PulseOximeter pox;
uint32_t tsLastReport = 0;

void onBeatDetected()
{
    Serial.println("Beat Detected!");
}
void setup() {
  Serial.begin(115200); 
   Serial.print("Initializing pulse oximeter..");
    // Initialize the PulseOximeter instance
    // Failures are generally due to an improper I2C wiring, missing power supply
    // or wrong target chip
    if (!pox.begin()) {
        Serial.println("FAILED");
        for(;;);
    } else {
        Serial.println("SUCCESS");
    }
     pox.setIRLedCurrent(MAX30100_LED_CURR_14_2MA);
 
    // Register a callback for the beat detection
    pox.setOnBeatDetectedCallback(onBeatDetected);
 
  WiFi.begin(ssid, password); // Attempt to connect to wifi with our password
  Serial.println("Connecting"); // Print our status to the serial monitor
  // Wait for wifi to connect
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}
 
 
void loop() {

  int spo2 = pox.getSpO2();
  int bpm = pox.getHeartRate();
      pox.update();
    if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
        Serial.print("Heart rate:");
        Serial.print(pox.getHeartRate());
        Serial.print("bpm / SpO2:");
        Serial.print(pox.getSpO2());
        Serial.println("%");
 
        tsLastReport = millis();
    }
   
  if ((millis() - lastTime) > timerDelay) { // Check if its been a minute
    if(WiFi.status()== WL_CONNECTED){ // Check to make sure wifi is still connected
      sendData(spo2, bpm); // Call the sendData function defined below
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
}
 
 
void sendData(int spo2, int bpm){
  HTTPClient http; // Initialize our HTTP client
 
 
  String url = serverName + "&field1=" + spo2+ "&field2="+ bpm; // Define our entire url
       
  http.begin(url.c_str()); // Initialize our HTTP request
       
  int httpResponseCode = http.GET(); // Send HTTP request
       
  if (httpResponseCode > 0){ // Check for good HTTP status code
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
  }else{
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}
