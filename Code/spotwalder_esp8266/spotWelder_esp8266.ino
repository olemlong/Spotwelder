
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include <OneWire.h> // bus com for sensors
#include <DallasTemperature.h> // protocol for temperaturesensors
#include "SSD1306Brzo.h"

SSD1306Brzo display(0x3c, D3, D5);
#define VERSION "1.0"

const char* ssid = "YOUR WIFI SSID HERE!";
const char* password = "THE WIFI PASSWORD";

#define potPin 0   // analog pin used to connect the potentiometer
int potVal;       // variable to read the value from the analog pin

#define inPin 16
int triggerSwitch = 0;
#define ssrPin 13 //Solid State Relay pin output

#define ONE_WIRE_BUS 4
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

void setup(void)
{
  Serial.begin(115200);
  OtaSetup();
  digitalWrite(inPin, LOW);
  // Initialising the UI will init the display too.
  display.init();

  display.flipScreenVertically();

  sensors.begin(); //start temp sensor
  
  Serial.println("Starting Spot Welder...");
  pinMode(inPin, INPUT);
   
  pinMode(ssrPin, OUTPUT);
 
  displayIntro();
  Serial.println("Running!");

}

void loop(void) {
   ArduinoOTA.handle();
  sensors.requestTemperatures(); // Send the command to get temperatures
  double temperature = sensors.getTempCByIndex(0);
  // reads the value of the potentiometer (value between 0 and 1023)
  potVal = map(analogRead(potPin), 0, 1023, 10, 500);     // scale it to use it get the right time (value between 10 and 500)
  display.normalDisplay();
  display.setFont(ArialMT_Plain_24);
  display.drawString(30, 9, String(potVal) += "ms");
  display.setFont(ArialMT_Plain_10);
  display.drawString(22, 35, "Transformer temp:");
  display.drawString(35, 0, "Welding time:");

  display.setFont(ArialMT_Plain_16);

  display.drawString(42, 47, String(temperature) += "C");
  display.display();
 
      triggerSwitch = digitalRead(inPin);

      //when weld button is pressed
      if (triggerSwitch == HIGH) {
        display.clear();
        display.invertDisplay();
        display.setFont(ArialMT_Plain_24);
        display.drawString(15, 3, "Welding!!");
        display.display();
        digitalWrite(ssrPin, HIGH);
        delay(potVal);
        digitalWrite(ssrPin, LOW);
        delay(1000);        
      }
  display.clear();
}
void displayIntro() {
  display.setFont(ArialMT_Plain_10);
  display.drawString(60, 21, VERSION);
  display.drawString(49, 40, "Longva");
  display.drawString(70, 50, "Engineering");

  display.setFont(ArialMT_Plain_16);
  display.drawString(20, 3, "Spot Welder");
  display.display();
  delay(2000);
  display.clear();
  display.display();

}

void OtaSetup() {  
  Serial.println("Booting Wifi system...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

   ArduinoOTA.setHostname("SpotWelder");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}
