#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include "DHT.h"
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

#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, HAIO_SERVER, HAIO_SERVERPORT, HAIO_USERNAME, HAIO_KEY);

// Setup a feed called 'temp' and 'fukt' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish temp = Adafruit_MQTT_Publish(&mqtt, "eiendom/hus/kneloft/temp");
Adafruit_MQTT_Publish fukt = Adafruit_MQTT_Publish(&mqtt, "eiendom/hus/kneloft/fukt");

void setup(void){
  Serial.begin(115200);
  dht.begin();
  Serial.println(""); 
}

void loop(void){
  // Connect to Wifi, could be lost after sleep
  initWifi();
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  //Read values
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Now we can publish stuff!
  Serial.print("Sending temp and moist val");
  if (! temp.publish(t)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
    Serial.println(t);
  }
  if (! fukt.publish(h)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
    Serial.println(h);
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

void tempStatus() {
    float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

String message = "Current readings are: ";
message += "Humidity: ";
message += h;
message += " %, ";
message += "Temperature: ";
message += t;
message += " C, ";
message += "Heat index: ";
message += hic;
message += " C.";
char charbuf[message.length()+1];
message.toCharArray(charbuf,message.length());

}
