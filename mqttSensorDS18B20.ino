#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include "OneWire.h"
#include "DallasTemperature.h"
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "credentials.h"

// WIFI AND MQTT information is stored in credentials.h 
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

const char* HAIO_SERVER = MQTT_SERVER;
const int HAIO_SERVERPORT = MQTT_SERVERPORT;
const char* HAIO_USERNAME = MQTT_USERNAME;
const char* HAIO_KEY = MQTT_KEY;


// Data wire is connteced to pin 2
#define ONE_WIRE_BUS 2
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature DS18B20(&oneWire);

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
//WiFiClient client;
// or... use WiFiFlientSecure for SSL
WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, HAIO_SERVER, HAIO_SERVERPORT, HAIO_USERNAME, HAIO_KEY);

// io.adafruit.com SHA1 fingerprint
static const char *fingerprint PROGMEM = "59 3C 48 0A B1 8B 39 4E 0D 58 50 47 9A 13 55 60 CC A0 1D AF";

// Setup a feed called 'temp' and 'fukt' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish temp = Adafruit_MQTT_Publish(&mqtt, MQTT_USERNAME "/feeds/huss-krypkjeller-temp");

void setup(void){
  Serial.begin(115200);
  DS18B20.begin();
  Serial.println(""); 
  // check the fingerprint of io.adafruit.com's SSL cert
  client.setFingerprint(fingerprint);
}

void loop(void){
  // Connect to Wifi, could be lost after sleep
  initWifi();
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();
  
  //Read values
  DS18B20.requestTemperatures();
  float t = DS18B20.getTempCByIndex(0);
  // Check if any reads failed and exit early (to try again).
  if (isnan(t)) {
    Serial.println("Failed to read from sensor!");
    return;
  }

  // Now we can publish stuff!
  Serial.print("Sending temperature");
  if (! temp.publish(t)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
    Serial.println(t);
  }
  
//sleep wifi
//WiFi.mode(WIFI_OFF);
//WiFi.forceSleepBegin();
//delay(1);
//wait 60 sek
delay(60000);
Serial.println("Wake");
// Wake up wifi
//WiFi.forceSleepWake(); 
//delay(1);

}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}

void initWifi() {
  //stop if already connected
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
    // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}
